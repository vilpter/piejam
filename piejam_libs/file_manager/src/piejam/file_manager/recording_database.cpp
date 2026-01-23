// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording_database.h>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>

namespace piejam::file_manager
{

namespace
{

auto
time_point_to_string(std::chrono::system_clock::time_point tp) -> std::string
{
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&time_t_val));
    return buf;
}

auto
string_to_time_point(std::string const& str) -> std::chrono::system_clock::time_point
{
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

auto
recording_metadata_to_json(recording_metadata const& m) -> QJsonObject
{
    QJsonObject obj;
    obj["file_path"] = QString::fromStdString(m.file_path);
    obj["filename"] = QString::fromStdString(m.filename);
    obj["file_hash"] = QString::fromStdString(m.file_hash);
    obj["file_size"] = static_cast<qint64>(m.file_size);
    obj["duration_ms"] = static_cast<qint64>(m.duration_ms);
    obj["sample_rate"] = static_cast<int>(m.sample_rate);
    obj["bit_depth"] = m.bit_depth;
    obj["channels"] = m.channels;
    obj["format"] = static_cast<int>(m.format);
    obj["peak_level_db"] = static_cast<double>(m.peak_level_db);
    obj["rms_level_db"] = static_cast<double>(m.rms_level_db);
    obj["has_clipping"] = m.has_clipping;
    obj["clip_count"] = m.clip_count;
    obj["recorded_at"] = QString::fromStdString(time_point_to_string(m.recorded_at));
    obj["last_modified"] = QString::fromStdString(time_point_to_string(m.last_modified));
    obj["storage_location"] = QString::fromStdString(m.storage_location);
    return obj;
}

auto
json_to_recording_metadata(QJsonObject const& obj) -> recording_metadata
{
    recording_metadata m;
    m.file_path = obj["file_path"].toString().toStdString();
    m.filename = obj["filename"].toString().toStdString();
    m.file_hash = obj["file_hash"].toString().toStdString();
    m.file_size = static_cast<uint64_t>(obj["file_size"].toVariant().toLongLong());
    m.duration_ms = static_cast<uint64_t>(obj["duration_ms"].toVariant().toLongLong());
    m.sample_rate = static_cast<uint32_t>(obj["sample_rate"].toInt());
    m.bit_depth = static_cast<uint16_t>(obj["bit_depth"].toInt());
    m.channels = static_cast<uint16_t>(obj["channels"].toInt());
    m.format = static_cast<audio_format>(obj["format"].toInt());
    m.peak_level_db = static_cast<float>(obj["peak_level_db"].toDouble());
    m.rms_level_db = static_cast<float>(obj["rms_level_db"].toDouble());
    m.has_clipping = obj["has_clipping"].toBool();
    m.clip_count = obj["clip_count"].toInt();
    m.recorded_at = string_to_time_point(obj["recorded_at"].toString().toStdString());
    m.last_modified = string_to_time_point(obj["last_modified"].toString().toStdString());
    m.storage_location = obj["storage_location"].toString().toStdString();
    return m;
}

auto
user_metadata_to_json(user_metadata const& m) -> QJsonObject
{
    QJsonObject obj;
    obj["file_hash"] = QString::fromStdString(m.file_hash);

    QJsonArray tags_array;
    for (auto const& tag : m.tags)
    {
        tags_array.append(QString::fromStdString(tag));
    }
    obj["tags"] = tags_array;

    obj["notes"] = QString::fromStdString(m.notes);
    obj["rating"] = m.rating;
    obj["status"] = static_cast<int>(m.status);
    obj["project_name"] = QString::fromStdString(m.project_name);
    obj["artist_names"] = QString::fromStdString(m.artist_names);
    obj["location_venue"] = QString::fromStdString(m.location_venue);
    obj["created_at"] = QString::fromStdString(time_point_to_string(m.created_at));
    obj["modified_at"] = QString::fromStdString(time_point_to_string(m.modified_at));
    return obj;
}

auto
json_to_user_metadata(QJsonObject const& obj) -> user_metadata
{
    user_metadata m;
    m.file_hash = obj["file_hash"].toString().toStdString();

    QJsonArray tags_array = obj["tags"].toArray();
    for (auto const& tag : tags_array)
    {
        m.tags.push_back(tag.toString().toStdString());
    }

    m.notes = obj["notes"].toString().toStdString();
    m.rating = obj["rating"].toInt();
    m.status = static_cast<export_status>(obj["status"].toInt());
    m.project_name = obj["project_name"].toString().toStdString();
    m.artist_names = obj["artist_names"].toString().toStdString();
    m.location_venue = obj["location_venue"].toString().toStdString();
    m.created_at = string_to_time_point(obj["created_at"].toString().toStdString());
    m.modified_at = string_to_time_point(obj["modified_at"].toString().toStdString());
    return m;
}

} // anonymous namespace

struct recording_database::impl
{
    std::string db_path;
    std::map<std::string, recording_metadata> recordings; // keyed by file_hash
    std::map<std::string, user_metadata> user_data;       // keyed by file_hash
    std::string last_error;
};

recording_database::recording_database(std::string const& db_path)
    : m_impl(std::make_unique<impl>())
{
    m_impl->db_path = db_path;
    load();
}

recording_database::~recording_database()
{
    save();
}

recording_database::recording_database(recording_database&&) noexcept = default;
recording_database& recording_database::operator=(recording_database&&) noexcept = default;

auto
recording_database::load() -> bool
{
    QFile file(QString::fromStdString(m_impl->db_path));
    if (!file.exists())
    {
        return true; // Empty database is valid
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        m_impl->last_error = "Cannot open database file for reading";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);
    if (doc.isNull())
    {
        m_impl->last_error = parse_error.errorString().toStdString();
        return false;
    }

    QJsonObject root = doc.object();

    // Load recordings
    m_impl->recordings.clear();
    QJsonArray recordings_array = root["recordings"].toArray();
    for (auto const& rec : recordings_array)
    {
        auto metadata = json_to_recording_metadata(rec.toObject());
        m_impl->recordings[metadata.file_hash] = metadata;
    }

    // Load user data
    m_impl->user_data.clear();
    QJsonArray user_array = root["user_data"].toArray();
    for (auto const& usr : user_array)
    {
        auto metadata = json_to_user_metadata(usr.toObject());
        m_impl->user_data[metadata.file_hash] = metadata;
    }

    return true;
}

auto
recording_database::save() -> bool
{
    // Ensure directory exists
    QFileInfo file_info(QString::fromStdString(m_impl->db_path));
    QDir dir = file_info.dir();
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    QJsonObject root;

    // Save recordings
    QJsonArray recordings_array;
    for (auto const& [hash, metadata] : m_impl->recordings)
    {
        recordings_array.append(recording_metadata_to_json(metadata));
    }
    root["recordings"] = recordings_array;

    // Save user data
    QJsonArray user_array;
    for (auto const& [hash, metadata] : m_impl->user_data)
    {
        user_array.append(user_metadata_to_json(metadata));
    }
    root["user_data"] = user_array;

    QJsonDocument doc(root);

    QFile file(QString::fromStdString(m_impl->db_path));
    if (!file.open(QIODevice::WriteOnly))
    {
        m_impl->last_error = "Cannot open database file for writing";
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

auto
recording_database::exists() const -> bool
{
    return QFile::exists(QString::fromStdString(m_impl->db_path));
}

auto
recording_database::path() const -> std::string
{
    return m_impl->db_path;
}

void
recording_database::set_recording_metadata(recording_metadata const& metadata)
{
    m_impl->recordings[metadata.file_hash] = metadata;
}

auto
recording_database::get_recording_metadata(std::string const& file_hash)
    -> std::optional<recording_metadata>
{
    auto it = m_impl->recordings.find(file_hash);
    if (it != m_impl->recordings.end())
    {
        return it->second;
    }
    return std::nullopt;
}

auto
recording_database::get_recording_metadata_by_path(std::string const& file_path)
    -> std::optional<recording_metadata>
{
    for (auto const& [hash, metadata] : m_impl->recordings)
    {
        if (metadata.file_path == file_path)
        {
            return metadata;
        }
    }
    return std::nullopt;
}

auto
recording_database::remove_recording_metadata(std::string const& file_hash) -> bool
{
    return m_impl->recordings.erase(file_hash) > 0;
}

auto
recording_database::all_recording_metadata() -> std::vector<recording_metadata>
{
    std::vector<recording_metadata> result;
    result.reserve(m_impl->recordings.size());
    for (auto const& [hash, metadata] : m_impl->recordings)
    {
        result.push_back(metadata);
    }
    return result;
}

void
recording_database::set_user_metadata(user_metadata const& metadata)
{
    m_impl->user_data[metadata.file_hash] = metadata;
}

auto
recording_database::get_user_metadata(std::string const& file_hash)
    -> std::optional<user_metadata>
{
    auto it = m_impl->user_data.find(file_hash);
    if (it != m_impl->user_data.end())
    {
        return it->second;
    }
    return std::nullopt;
}

auto
recording_database::remove_user_metadata(std::string const& file_hash) -> bool
{
    return m_impl->user_data.erase(file_hash) > 0;
}

auto
recording_database::all_user_metadata() -> std::vector<user_metadata>
{
    std::vector<user_metadata> result;
    result.reserve(m_impl->user_data.size());
    for (auto const& [hash, metadata] : m_impl->user_data)
    {
        result.push_back(metadata);
    }
    return result;
}

auto
recording_database::get_recording_info(std::string const& file_hash)
    -> std::optional<recording_info>
{
    auto rec_opt = get_recording_metadata(file_hash);
    if (!rec_opt)
    {
        return std::nullopt;
    }

    recording_info info;
    info.metadata = *rec_opt;

    auto user_opt = get_user_metadata(file_hash);
    if (user_opt)
    {
        info.user_data = *user_opt;
    }
    else
    {
        info.user_data.file_hash = file_hash;
        info.user_data.created_at = std::chrono::system_clock::now();
        info.user_data.modified_at = info.user_data.created_at;
    }

    return info;
}

auto
recording_database::all_recordings() -> std::vector<recording_info>
{
    std::vector<recording_info> result;
    result.reserve(m_impl->recordings.size());

    for (auto const& [hash, metadata] : m_impl->recordings)
    {
        recording_info info;
        info.metadata = metadata;

        auto user_opt = get_user_metadata(hash);
        if (user_opt)
        {
            info.user_data = *user_opt;
        }
        else
        {
            info.user_data.file_hash = hash;
        }

        result.push_back(std::move(info));
    }

    // Sort by recording date, newest first
    std::sort(result.begin(), result.end(),
              [](recording_info const& a, recording_info const& b) {
                  return a.metadata.recorded_at > b.metadata.recorded_at;
              });

    return result;
}

void
recording_database::update_tags(std::string const& file_hash,
                                std::vector<std::string> const& tags)
{
    auto& user = m_impl->user_data[file_hash];
    user.file_hash = file_hash;
    user.tags = tags;
    user.modified_at = std::chrono::system_clock::now();
}

auto
recording_database::all_tags() -> std::vector<std::string>
{
    std::set<std::string> unique_tags;
    for (auto const& [hash, user] : m_impl->user_data)
    {
        for (auto const& tag : user.tags)
        {
            unique_tags.insert(tag);
        }
    }
    return {unique_tags.begin(), unique_tags.end()};
}

auto
recording_database::recordings_with_tag(std::string const& tag) -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (auto const& [hash, user] : m_impl->user_data)
    {
        if (std::find(user.tags.begin(), user.tags.end(), tag) != user.tags.end())
        {
            result.push_back(hash);
        }
    }
    return result;
}

void
recording_database::update_notes(std::string const& file_hash, std::string const& notes)
{
    auto& user = m_impl->user_data[file_hash];
    user.file_hash = file_hash;
    user.notes = notes;
    user.modified_at = std::chrono::system_clock::now();
}

void
recording_database::update_rating(std::string const& file_hash, int rating)
{
    auto& user = m_impl->user_data[file_hash];
    user.file_hash = file_hash;
    user.rating = std::clamp(rating, 0, 5);
    user.modified_at = std::chrono::system_clock::now();
}

auto
recording_database::recordings_with_min_rating(int min_rating) -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (auto const& [hash, user] : m_impl->user_data)
    {
        if (user.rating >= min_rating)
        {
            result.push_back(hash);
        }
    }
    return result;
}

void
recording_database::update_export_status(std::string const& file_hash, export_status status)
{
    auto& user = m_impl->user_data[file_hash];
    user.file_hash = file_hash;
    user.status = status;
    user.modified_at = std::chrono::system_clock::now();
}

auto
recording_database::recordings_with_status(export_status status) -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (auto const& [hash, user] : m_impl->user_data)
    {
        if (user.status == status)
        {
            result.push_back(hash);
        }
    }
    return result;
}

void
recording_database::prune_deleted_files(std::vector<std::string> const& valid_hashes)
{
    std::set<std::string> valid_set(valid_hashes.begin(), valid_hashes.end());

    std::vector<std::string> to_remove;
    for (auto const& [hash, _] : m_impl->recordings)
    {
        if (valid_set.find(hash) == valid_set.end())
        {
            to_remove.push_back(hash);
        }
    }

    for (auto const& hash : to_remove)
    {
        m_impl->recordings.erase(hash);
        m_impl->user_data.erase(hash);
    }
}

auto
recording_database::recording_count() const -> size_t
{
    return m_impl->recordings.size();
}

auto
recording_database::last_error() const -> std::string
{
    return m_impl->last_error;
}

} // namespace piejam::file_manager
