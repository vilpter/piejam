// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording_list_model.h>

#include <QDateTime>

#include <algorithm>

namespace piejam::file_manager
{

struct RecordingListModel::impl
{
    std::vector<recording_info> recordings;
};

RecordingListModel::RecordingListModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_impl(std::make_unique<impl>())
{
}

RecordingListModel::~RecordingListModel() = default;

int
RecordingListModel::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_impl->recordings.size());
}

QVariant
RecordingListModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_impl->recordings.size()))
    {
        return {};
    }

    auto const& info = m_impl->recordings[static_cast<size_t>(row)];
    auto const& meta = info.metadata;
    auto const& user = info.user_data;

    switch (role)
    {
    case FileHashRole:
        return QString::fromStdString(meta.file_hash);

    case FilePathRole:
        return QString::fromStdString(meta.file_path);

    case FilenameRole:
        return QString::fromStdString(meta.filename);

    case DurationRole:
        return static_cast<qint64>(meta.duration_ms);

    case DurationStringRole:
        return QString::fromStdString(info.duration_string());

    case SampleRateRole:
        return static_cast<int>(meta.sample_rate);

    case BitDepthRole:
        return meta.bit_depth;

    case ChannelsRole:
        return meta.channels;

    case FormatRole:
        return static_cast<int>(meta.format);

    case FormatStringRole:
        return QString::fromUtf8(audio_format_to_string(meta.format));

    case FileSizeRole:
        return static_cast<qint64>(meta.file_size);

    case FileSizeStringRole:
        return QString::fromStdString(info.file_size_string());

    case RecordedAtRole:
    {
        auto time_t_val = std::chrono::system_clock::to_time_t(meta.recorded_at);
        return QDateTime::fromSecsSinceEpoch(time_t_val);
    }

    case RecordedAtStringRole:
    {
        auto time_t_val = std::chrono::system_clock::to_time_t(meta.recorded_at);
        QDateTime dt = QDateTime::fromSecsSinceEpoch(time_t_val);

        // Human-readable relative time
        QDateTime now = QDateTime::currentDateTime();
        qint64 secs = dt.secsTo(now);

        if (secs < 60)
        {
            return QStringLiteral("Just now");
        }
        if (secs < 3600)
        {
            int mins = static_cast<int>(secs / 60);
            return QString("%1 minute%2 ago").arg(mins).arg(mins == 1 ? "" : "s");
        }
        if (secs < 86400)
        {
            int hours = static_cast<int>(secs / 3600);
            return QString("%1 hour%2 ago").arg(hours).arg(hours == 1 ? "" : "s");
        }
        if (secs < 172800)
        {
            return QStringLiteral("Yesterday");
        }
        if (secs < 604800)
        {
            int days = static_cast<int>(secs / 86400);
            return QString("%1 days ago").arg(days);
        }

        return dt.toString("MMM d, yyyy");
    }

    case StorageLocationRole:
        return QString::fromStdString(meta.storage_location);

    case PeakLevelDbRole:
        return static_cast<double>(meta.peak_level_db);

    case RmsLevelDbRole:
        return static_cast<double>(meta.rms_level_db);

    case HasClippingRole:
        return meta.has_clipping;

    case TagsRole:
    {
        QStringList tags;
        for (auto const& tag : user.tags)
        {
            tags.append(QString::fromStdString(tag));
        }
        return tags;
    }

    case NotesRole:
        return QString::fromStdString(user.notes);

    case RatingRole:
        return user.rating;

    case ExportStatusRole:
        return static_cast<int>(user.status);

    case ExportStatusStringRole:
        return QString::fromUtf8(export_status_to_string(user.status));

    case ProjectNameRole:
        return QString::fromStdString(user.project_name);

    case ArtistNamesRole:
        return QString::fromStdString(user.artist_names);

    case LocationVenueRole:
        return QString::fromStdString(user.location_venue);

    default:
        return {};
    }
}

QHash<int, QByteArray>
RecordingListModel::roleNames() const
{
    return {
        {FileHashRole, "fileHash"},
        {FilePathRole, "filePath"},
        {FilenameRole, "filename"},
        {DurationRole, "duration"},
        {DurationStringRole, "durationString"},
        {SampleRateRole, "sampleRate"},
        {BitDepthRole, "bitDepth"},
        {ChannelsRole, "channels"},
        {FormatRole, "format"},
        {FormatStringRole, "formatString"},
        {FileSizeRole, "fileSize"},
        {FileSizeStringRole, "fileSizeString"},
        {RecordedAtRole, "recordedAt"},
        {RecordedAtStringRole, "recordedAtString"},
        {StorageLocationRole, "storageLocation"},
        {PeakLevelDbRole, "peakLevelDb"},
        {RmsLevelDbRole, "rmsLevelDb"},
        {HasClippingRole, "hasClipping"},
        {TagsRole, "tags"},
        {NotesRole, "notes"},
        {RatingRole, "rating"},
        {ExportStatusRole, "exportStatus"},
        {ExportStatusStringRole, "exportStatusString"},
        {ProjectNameRole, "projectName"},
        {ArtistNamesRole, "artistNames"},
        {LocationVenueRole, "locationVenue"}
    };
}

void
RecordingListModel::setRecordings(std::vector<recording_info> recordings)
{
    beginResetModel();
    m_impl->recordings = std::move(recordings);
    endResetModel();
}

auto
RecordingListModel::recordings() const -> std::vector<recording_info> const&
{
    return m_impl->recordings;
}

auto
RecordingListModel::recordingAt(int index) const -> recording_info const*
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return nullptr;
    }
    return &m_impl->recordings[static_cast<size_t>(index)];
}

auto
RecordingListModel::indexForHash(std::string const& hash) const -> int
{
    for (size_t i = 0; i < m_impl->recordings.size(); ++i)
    {
        if (m_impl->recordings[i].metadata.file_hash == hash)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void
RecordingListModel::updateRecording(int index, recording_info const& info)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    m_impl->recordings[static_cast<size_t>(index)] = info;

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit recordingUpdated(index);
}

void
RecordingListModel::removeRecording(int index)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    QString fileHash = QString::fromStdString(
        m_impl->recordings[static_cast<size_t>(index)].metadata.file_hash);

    beginRemoveRows(QModelIndex(), index, index);
    m_impl->recordings.erase(m_impl->recordings.begin() + index);
    endRemoveRows();

    emit recordingRemoved(fileHash);
}

void
RecordingListModel::addRecording(recording_info const& info)
{
    int index = static_cast<int>(m_impl->recordings.size());

    beginInsertRows(QModelIndex(), index, index);
    m_impl->recordings.push_back(info);
    endInsertRows();

    emit recordingAdded(index);
}

void
RecordingListModel::clear()
{
    beginResetModel();
    m_impl->recordings.clear();
    endResetModel();
}

void
RecordingListModel::updateTags(int index, QStringList const& tags)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    auto& user = m_impl->recordings[static_cast<size_t>(index)].user_data;
    user.tags.clear();
    for (auto const& tag : tags)
    {
        user.tags.push_back(tag.toStdString());
    }
    user.modified_at = std::chrono::system_clock::now();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {TagsRole});
    emit recordingUpdated(index);
}

void
RecordingListModel::updateNotes(int index, QString const& notes)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    auto& user = m_impl->recordings[static_cast<size_t>(index)].user_data;
    user.notes = notes.toStdString();
    user.modified_at = std::chrono::system_clock::now();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {NotesRole});
    emit recordingUpdated(index);
}

void
RecordingListModel::updateRating(int index, int rating)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    auto& user = m_impl->recordings[static_cast<size_t>(index)].user_data;
    user.rating = std::clamp(rating, 0, 5);
    user.modified_at = std::chrono::system_clock::now();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {RatingRole});
    emit recordingUpdated(index);
}

void
RecordingListModel::updateExportStatus(int index, int status)
{
    if (index < 0 || index >= static_cast<int>(m_impl->recordings.size()))
    {
        return;
    }

    auto& user = m_impl->recordings[static_cast<size_t>(index)].user_data;
    user.status = static_cast<export_status>(status);
    user.modified_at = std::chrono::system_clock::now();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {ExportStatusRole, ExportStatusStringRole});
    emit recordingUpdated(index);
}

} // namespace piejam::file_manager
