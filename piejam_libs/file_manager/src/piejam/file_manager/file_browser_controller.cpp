// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/file_browser_controller.h>

#include <piejam/file_manager/metadata_extractor.h>
#include <piejam/file_manager/recording_database.h>
#include <piejam/file_manager/recording_list_model.h>
#include <piejam/file_manager/recording_scanner.h>

#include <QDir>
#include <QFile>
#include <QStorageInfo>
#include <QtConcurrent>

#include <algorithm>

namespace piejam::file_manager
{

struct FileBrowserController::impl
{
    std::string recordings_path;
    std::unique_ptr<recording_scanner> scanner;
    std::unique_ptr<metadata_extractor> extractor;
    std::unique_ptr<recording_database> database;
    std::unique_ptr<RecordingListModel> model;

    bool is_scanning{false};
    qint64 used_bytes{0};
    qint64 total_bytes{0};

    // Filtering
    std::string filter_tag;
    int filter_min_rating{0};
    export_status filter_status{export_status::new_recording};
    bool has_status_filter{false};

    std::vector<recording_info> all_recordings;
};

FileBrowserController::FileBrowserController(
    std::string const& recordings_path,
    std::string const& db_path,
    QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<impl>())
{
    m_impl->recordings_path = recordings_path;
    m_impl->scanner = std::make_unique<recording_scanner>(this);
    m_impl->extractor = std::make_unique<metadata_extractor>();
    m_impl->database = std::make_unique<recording_database>(db_path);
    m_impl->model = std::make_unique<RecordingListModel>(this);

    // Add default recordings path
    m_impl->scanner->add_scan_directory(recordings_path);

    // Connect scanner signals
    connect(m_impl->scanner.get(), &recording_scanner::scanStarted,
            this, [this]() {
                m_impl->is_scanning = true;
                emit isScanningChanged();
            });

    connect(m_impl->scanner.get(), &recording_scanner::scanComplete,
            this, [this](int fileCount) {
                m_impl->is_scanning = false;
                emit isScanningChanged();
                emit scanComplete(fileCount);
                emit recordingCountChanged();
            });

    connect(m_impl->scanner.get(), &recording_scanner::scanProgress,
            this, &FileBrowserController::scanProgress);

    connect(m_impl->scanner.get(), &recording_scanner::newFileDetected,
            this, [this](QString const& filePath) {
                // Extract metadata and add to database
                auto metadata = m_impl->extractor->extract(filePath.toStdString());
                if (!metadata.file_hash.empty())
                {
                    m_impl->database->set_recording_metadata(metadata);

                    recording_info info;
                    info.metadata = metadata;
                    info.user_data.file_hash = metadata.file_hash;
                    info.user_data.created_at = std::chrono::system_clock::now();
                    info.user_data.modified_at = info.user_data.created_at;

                    m_impl->all_recordings.push_back(info);
                    applyFilters();

                    emit newRecordingDetected(QString::fromStdString(metadata.filename));
                    emit recordingCountChanged();
                }
            });

    // Enable auto-scan for new files
    m_impl->scanner->set_auto_scan(true);

    // Update storage info
    updateStorageInfo();

    // Initial scan
    refresh();
}

FileBrowserController::~FileBrowserController()
{
    m_impl->database->save();
}

FileBrowserController::FileBrowserController(FileBrowserController&&) noexcept = default;
FileBrowserController& FileBrowserController::operator=(FileBrowserController&&) noexcept = default;

RecordingListModel*
FileBrowserController::recordings() const
{
    return m_impl->model.get();
}

QString
FileBrowserController::recordingsPath() const
{
    return QString::fromStdString(m_impl->recordings_path);
}

void
FileBrowserController::setRecordingsPath(QString const& path)
{
    std::string new_path = path.toStdString();
    if (new_path != m_impl->recordings_path)
    {
        m_impl->scanner->remove_scan_directory(m_impl->recordings_path);
        m_impl->recordings_path = new_path;
        m_impl->scanner->add_scan_directory(new_path);
        emit recordingsPathChanged();
        rescan();
    }
}

QString
FileBrowserController::storageInfo() const
{
    if (m_impl->total_bytes == 0)
    {
        return QStringLiteral("Unknown storage");
    }

    double used_gb = m_impl->used_bytes / (1024.0 * 1024.0 * 1024.0);
    double total_gb = m_impl->total_bytes / (1024.0 * 1024.0 * 1024.0);
    int percent = usedPercent();

    return QString("%1 GB / %2 GB used (%3%)")
        .arg(used_gb, 0, 'f', 1)
        .arg(total_gb, 0, 'f', 1)
        .arg(percent);
}

qint64
FileBrowserController::usedBytes() const
{
    return m_impl->used_bytes;
}

qint64
FileBrowserController::totalBytes() const
{
    return m_impl->total_bytes;
}

int
FileBrowserController::usedPercent() const
{
    if (m_impl->total_bytes == 0)
    {
        return 0;
    }
    return static_cast<int>((m_impl->used_bytes * 100) / m_impl->total_bytes);
}

bool
FileBrowserController::isScanning() const
{
    return m_impl->is_scanning;
}

int
FileBrowserController::recordingCount() const
{
    return static_cast<int>(m_impl->all_recordings.size());
}

QStringList
FileBrowserController::allTags() const
{
    QStringList result;
    auto tags = m_impl->database->all_tags();
    for (auto const& tag : tags)
    {
        result.append(QString::fromStdString(tag));
    }
    return result;
}

void
FileBrowserController::refresh()
{
    // Load from database
    m_impl->all_recordings = m_impl->database->all_recordings();
    applyFilters();
    updateStorageInfo();
}

void
FileBrowserController::rescan()
{
    m_impl->scanner->scan_async([this](std::vector<std::string> const& files) {
        // Process found files
        std::vector<std::string> valid_hashes;

        for (auto const& file_path : files)
        {
            // Check if already in database
            auto existing = m_impl->database->get_recording_metadata_by_path(file_path);
            if (existing)
            {
                valid_hashes.push_back(existing->file_hash);
                continue;
            }

            // Extract metadata for new file
            auto metadata = m_impl->extractor->extract(file_path);
            if (!metadata.file_hash.empty())
            {
                m_impl->database->set_recording_metadata(metadata);
                valid_hashes.push_back(metadata.file_hash);
            }
        }

        // Prune database of deleted files
        m_impl->database->prune_deleted_files(valid_hashes);
        m_impl->database->save();

        // Refresh display
        QMetaObject::invokeMethod(this, [this]() {
            refresh();
        }, Qt::QueuedConnection);
    });
}

void
FileBrowserController::deleteRecording(int index)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        emit operationError(QStringLiteral("Invalid recording index"));
        return;
    }

    QString file_path = QString::fromStdString(info->metadata.file_path);
    QString filename = QString::fromStdString(info->metadata.filename);
    std::string file_hash = info->metadata.file_hash;

    QFile file(file_path);
    if (!file.remove())
    {
        emit operationError(QString("Failed to delete file: %1").arg(file.errorString()));
        return;
    }

    // Remove from database
    m_impl->database->remove_recording_metadata(file_hash);
    m_impl->database->remove_user_metadata(file_hash);
    m_impl->database->save();

    // Remove from all_recordings
    auto it = std::find_if(m_impl->all_recordings.begin(), m_impl->all_recordings.end(),
                           [&file_hash](recording_info const& r) {
                               return r.metadata.file_hash == file_hash;
                           });
    if (it != m_impl->all_recordings.end())
    {
        m_impl->all_recordings.erase(it);
    }

    // Remove from model
    m_impl->model->removeRecording(index);

    updateStorageInfo();
    emit recordingDeleted(index, filename);
    emit recordingCountChanged();
}

void
FileBrowserController::updateTags(int index, QStringList const& tags)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        return;
    }

    std::vector<std::string> tag_vec;
    for (auto const& tag : tags)
    {
        tag_vec.push_back(tag.toStdString());
    }

    m_impl->database->update_tags(info->metadata.file_hash, tag_vec);
    m_impl->database->save();

    m_impl->model->updateTags(index, tags);

    // Update in all_recordings
    for (auto& rec : m_impl->all_recordings)
    {
        if (rec.metadata.file_hash == info->metadata.file_hash)
        {
            rec.user_data.tags = tag_vec;
            break;
        }
    }

    emit tagsChanged();
}

void
FileBrowserController::updateNotes(int index, QString const& notes)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        return;
    }

    m_impl->database->update_notes(info->metadata.file_hash, notes.toStdString());
    m_impl->database->save();

    m_impl->model->updateNotes(index, notes);

    // Update in all_recordings
    for (auto& rec : m_impl->all_recordings)
    {
        if (rec.metadata.file_hash == info->metadata.file_hash)
        {
            rec.user_data.notes = notes.toStdString();
            break;
        }
    }
}

void
FileBrowserController::updateRating(int index, int rating)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        return;
    }

    m_impl->database->update_rating(info->metadata.file_hash, rating);
    m_impl->database->save();

    m_impl->model->updateRating(index, rating);

    // Update in all_recordings
    for (auto& rec : m_impl->all_recordings)
    {
        if (rec.metadata.file_hash == info->metadata.file_hash)
        {
            rec.user_data.rating = rating;
            break;
        }
    }
}

void
FileBrowserController::updateExportStatus(int index, int status)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        return;
    }

    auto export_stat = static_cast<export_status>(status);
    m_impl->database->update_export_status(info->metadata.file_hash, export_stat);
    m_impl->database->save();

    m_impl->model->updateExportStatus(index, status);

    // Update in all_recordings
    for (auto& rec : m_impl->all_recordings)
    {
        if (rec.metadata.file_hash == info->metadata.file_hash)
        {
            rec.user_data.status = export_stat;
            break;
        }
    }
}

void
FileBrowserController::verifyIntegrity(int index)
{
    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        emit integrityCheckComplete(index, false, QStringLiteral("Invalid recording index"));
        return;
    }

    std::string file_path = info->metadata.file_path;
    std::string stored_hash = info->metadata.file_hash;

    QtConcurrent::run([this, index, file_path, stored_hash]() {
        std::string computed_hash = m_impl->extractor->calculate_hash(file_path);

        bool passed = (computed_hash == stored_hash);
        QString message = passed
            ? QStringLiteral("File integrity verified successfully")
            : QStringLiteral("File integrity check failed - file may have been modified");

        QMetaObject::invokeMethod(this, [this, index, passed, message]() {
            emit integrityCheckComplete(index, passed, message);
        }, Qt::QueuedConnection);
    });
}

void
FileBrowserController::addScanDirectory(QString const& path)
{
    m_impl->scanner->add_scan_directory(path.toStdString());
}

void
FileBrowserController::removeScanDirectory(QString const& path)
{
    m_impl->scanner->remove_scan_directory(path.toStdString());
}

QStringList
FileBrowserController::scanDirectories() const
{
    QStringList result;
    for (auto const& dir : m_impl->scanner->scan_directories())
    {
        result.append(QString::fromStdString(dir));
    }
    return result;
}

void
FileBrowserController::filterByTag(QString const& tag)
{
    m_impl->filter_tag = tag.toStdString();
    applyFilters();
}

void
FileBrowserController::filterByMinRating(int minRating)
{
    m_impl->filter_min_rating = minRating;
    applyFilters();
}

void
FileBrowserController::filterByStatus(int status)
{
    m_impl->filter_status = static_cast<export_status>(status);
    m_impl->has_status_filter = true;
    applyFilters();
}

void
FileBrowserController::clearFilters()
{
    m_impl->filter_tag.clear();
    m_impl->filter_min_rating = 0;
    m_impl->has_status_filter = false;
    applyFilters();
}

QVariantMap
FileBrowserController::getRecordingDetails(int index) const
{
    QVariantMap result;

    auto const* info = m_impl->model->recordingAt(index);
    if (!info)
    {
        return result;
    }

    auto const& meta = info->metadata;
    auto const& user = info->user_data;

    result["fileHash"] = QString::fromStdString(meta.file_hash);
    result["filePath"] = QString::fromStdString(meta.file_path);
    result["filename"] = QString::fromStdString(meta.filename);
    result["duration"] = static_cast<qint64>(meta.duration_ms);
    result["durationString"] = QString::fromStdString(info->duration_string());
    result["sampleRate"] = static_cast<int>(meta.sample_rate);
    result["bitDepth"] = meta.bit_depth;
    result["channels"] = meta.channels;
    result["format"] = static_cast<int>(meta.format);
    result["formatString"] = QString::fromUtf8(audio_format_to_string(meta.format));
    result["fileSize"] = static_cast<qint64>(meta.file_size);
    result["fileSizeString"] = QString::fromStdString(info->file_size_string());
    result["storageLocation"] = QString::fromStdString(meta.storage_location);
    result["peakLevelDb"] = static_cast<double>(meta.peak_level_db);
    result["rmsLevelDb"] = static_cast<double>(meta.rms_level_db);
    result["hasClipping"] = meta.has_clipping;
    result["clipCount"] = meta.clip_count;

    QStringList tags;
    for (auto const& tag : user.tags)
    {
        tags.append(QString::fromStdString(tag));
    }
    result["tags"] = tags;

    result["notes"] = QString::fromStdString(user.notes);
    result["rating"] = user.rating;
    result["exportStatus"] = static_cast<int>(user.status);
    result["exportStatusString"] = QString::fromUtf8(export_status_to_string(user.status));
    result["projectName"] = QString::fromStdString(user.project_name);
    result["artistNames"] = QString::fromStdString(user.artist_names);
    result["locationVenue"] = QString::fromStdString(user.location_venue);

    return result;
}

void
FileBrowserController::applyFilters()
{
    std::vector<recording_info> filtered;

    for (auto const& rec : m_impl->all_recordings)
    {
        // Tag filter
        if (!m_impl->filter_tag.empty())
        {
            auto const& tags = rec.user_data.tags;
            if (std::find(tags.begin(), tags.end(), m_impl->filter_tag) == tags.end())
            {
                continue;
            }
        }

        // Rating filter
        if (m_impl->filter_min_rating > 0 && rec.user_data.rating < m_impl->filter_min_rating)
        {
            continue;
        }

        // Status filter
        if (m_impl->has_status_filter && rec.user_data.status != m_impl->filter_status)
        {
            continue;
        }

        filtered.push_back(rec);
    }

    m_impl->model->setRecordings(std::move(filtered));
}

void
FileBrowserController::updateStorageInfo()
{
    QStorageInfo storage(QString::fromStdString(m_impl->recordings_path));
    if (storage.isValid())
    {
        m_impl->total_bytes = storage.bytesTotal();
        m_impl->used_bytes = storage.bytesTotal() - storage.bytesAvailable();
        emit storageInfoChanged();
    }
}

} // namespace piejam::file_manager
