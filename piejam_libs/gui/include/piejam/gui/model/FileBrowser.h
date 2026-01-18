// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>

#include <QObject>
#include <QStringList>
#include <QVariantMap>

#include <memory>

class QAbstractListModel;

namespace piejam::file_manager
{
class FileBrowserController;
} // namespace piejam::file_manager

namespace piejam::gui::model
{

/// QML-exposed model for file browser functionality.
///
/// Wraps the file_manager::FileBrowserController and exposes its
/// functionality to QML for the recordings browser UI.
class FileBrowser : public QObject
{
    Q_OBJECT

    // Storage info
    PIEJAM_GUI_PROPERTY(QString, storageInfo, setStorageInfo)
    PIEJAM_GUI_PROPERTY(qint64, usedBytes, setUsedBytes)
    PIEJAM_GUI_PROPERTY(qint64, totalBytes, setTotalBytes)
    PIEJAM_GUI_PROPERTY(int, usedPercent, setUsedPercent)

    // Status
    PIEJAM_GUI_PROPERTY(bool, isScanning, setIsScanning)
    PIEJAM_GUI_PROPERTY(int, recordingCount, setRecordingCount)
    PIEJAM_GUI_PROPERTY(QStringList, allTags, setAllTags)

    // Path
    PIEJAM_GUI_PROPERTY(QString, recordingsPath, setRecordingsPath)

public:
    explicit FileBrowser(
        std::string const& recordings_path,
        std::string const& db_path,
        QObject* parent = nullptr);

    ~FileBrowser() override;

    FileBrowser(FileBrowser const&) = delete;
    FileBrowser& operator=(FileBrowser const&) = delete;

    /// Get the recordings list model for QML binding
    Q_INVOKABLE QAbstractListModel* recordings() const;

    /// Get detailed info for a recording at index
    Q_INVOKABLE QVariantMap getRecordingDetails(int index) const;

    /// Refresh the recordings list
    Q_INVOKABLE void refresh();

    /// Rescan all directories
    Q_INVOKABLE void rescan();

    /// Delete a recording at index
    Q_INVOKABLE void deleteRecording(int index);

    /// Update tags for recording at index
    Q_INVOKABLE void updateTags(int index, QStringList const& tags);

    /// Update notes for recording at index
    Q_INVOKABLE void updateNotes(int index, QString const& notes);

    /// Update rating for recording at index (1-5, 0 = unrated)
    Q_INVOKABLE void updateRating(int index, int rating);

    /// Update export status for recording at index
    Q_INVOKABLE void updateExportStatus(int index, int status);

    /// Verify file integrity at index
    Q_INVOKABLE void verifyIntegrity(int index);

    /// Add additional scan directory (e.g., NFS mount)
    Q_INVOKABLE void addScanDirectory(QString const& path);

    /// Remove scan directory
    Q_INVOKABLE void removeScanDirectory(QString const& path);

    /// Get scan directories
    Q_INVOKABLE QStringList scanDirectories() const;

    /// Filter recordings by tag
    Q_INVOKABLE void filterByTag(QString const& tag);

    /// Filter recordings by minimum rating
    Q_INVOKABLE void filterByMinRating(int minRating);

    /// Filter recordings by export status
    Q_INVOKABLE void filterByStatus(int status);

    /// Clear all filters
    Q_INVOKABLE void clearFilters();

signals:
    void scanComplete(int fileCount);
    void scanProgress(int current, int total);
    void integrityCheckComplete(int index, bool passed, QString const& message);
    void recordingDeleted(int index, QString const& filename);
    void operationError(QString const& message);
    void newRecordingDetected(QString const& filename);

private:
    void connectSignals();
    void updateProperties();

    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::gui::model
