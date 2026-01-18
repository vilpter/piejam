// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/file_manager/fwd.h>
#include <piejam/file_manager/recording.h>

#include <QObject>

#include <memory>

namespace piejam::file_manager
{

/// Main controller for file browser functionality, exposed to QML
class FileBrowserController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RecordingListModel* recordings READ recordings CONSTANT)
    Q_PROPERTY(QString recordingsPath READ recordingsPath WRITE setRecordingsPath NOTIFY recordingsPathChanged)
    Q_PROPERTY(QString storageInfo READ storageInfo NOTIFY storageInfoChanged)
    Q_PROPERTY(qint64 usedBytes READ usedBytes NOTIFY storageInfoChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY storageInfoChanged)
    Q_PROPERTY(int usedPercent READ usedPercent NOTIFY storageInfoChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
    Q_PROPERTY(int recordingCount READ recordingCount NOTIFY recordingCountChanged)
    Q_PROPERTY(QStringList allTags READ allTags NOTIFY tagsChanged)

public:
    explicit FileBrowserController(
        std::string const& recordings_path,
        std::string const& db_path,
        QObject* parent = nullptr);
    ~FileBrowserController() override;

    FileBrowserController(FileBrowserController const&) = delete;
    FileBrowserController& operator=(FileBrowserController const&) = delete;
    FileBrowserController(FileBrowserController&&) noexcept;
    FileBrowserController& operator=(FileBrowserController&&) noexcept;

    /// Get recordings list model for QML
    RecordingListModel* recordings() const;

    /// Get current recordings path
    QString recordingsPath() const;

    /// Set recordings path
    void setRecordingsPath(QString const& path);

    /// Get storage info string (e.g., "45.2 GB / 64 GB used (70%)")
    QString storageInfo() const;

    /// Get used bytes
    qint64 usedBytes() const;

    /// Get total bytes
    qint64 totalBytes() const;

    /// Get used percentage
    int usedPercent() const;

    /// Check if scanning is in progress
    bool isScanning() const;

    /// Get number of recordings
    int recordingCount() const;

    /// Get all unique tags
    QStringList allTags() const;

public slots:
    /// Refresh the recordings list
    void refresh();

    /// Rescan all directories
    void rescan();

    /// Delete a recording at index
    void deleteRecording(int index);

    /// Update tags for recording at index
    void updateTags(int index, QStringList const& tags);

    /// Update notes for recording at index
    void updateNotes(int index, QString const& notes);

    /// Update rating for recording at index (1-5, 0 = unrated)
    void updateRating(int index, int rating);

    /// Update export status for recording at index
    void updateExportStatus(int index, int status);

    /// Verify file integrity at index
    void verifyIntegrity(int index);

    /// Add additional scan directory (e.g., NFS mount)
    void addScanDirectory(QString const& path);

    /// Remove scan directory
    void removeScanDirectory(QString const& path);

    /// Get scan directories
    QStringList scanDirectories() const;

    /// Filter recordings by tag
    void filterByTag(QString const& tag);

    /// Filter recordings by minimum rating
    void filterByMinRating(int minRating);

    /// Filter recordings by export status
    void filterByStatus(int status);

    /// Clear all filters
    void clearFilters();

    /// Get recording info as QVariantMap for detailed view
    QVariantMap getRecordingDetails(int index) const;

signals:
    void recordingsPathChanged();
    void storageInfoChanged();
    void isScanningChanged();
    void recordingCountChanged();
    void tagsChanged();
    void scanComplete(int fileCount);
    void scanProgress(int current, int total);
    void integrityCheckComplete(int index, bool passed, QString const& message);
    void recordingDeleted(int index, QString const& filename);
    void operationError(QString const& message);
    void newRecordingDetected(QString const& filename);

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::file_manager
