// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/file_manager/recording.h>

#include <QAbstractListModel>

#include <memory>
#include <vector>

namespace piejam::file_manager
{

/// Qt model for exposing recordings to QML
class RecordingListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        FileHashRole = Qt::UserRole + 1,
        FilePathRole,
        FilenameRole,
        DurationRole,
        DurationStringRole,
        SampleRateRole,
        BitDepthRole,
        ChannelsRole,
        FormatRole,
        FormatStringRole,
        FileSizeRole,
        FileSizeStringRole,
        RecordedAtRole,
        RecordedAtStringRole,
        StorageLocationRole,
        PeakLevelDbRole,
        RmsLevelDbRole,
        HasClippingRole,
        TagsRole,
        NotesRole,
        RatingRole,
        ExportStatusRole,
        ExportStatusStringRole,
        ProjectNameRole,
        ArtistNamesRole,
        LocationVenueRole
    };
    Q_ENUM(Roles)

    explicit RecordingListModel(QObject* parent = nullptr);
    ~RecordingListModel() override;

    RecordingListModel(RecordingListModel const&) = delete;
    RecordingListModel& operator=(RecordingListModel const&) = delete;

    // QAbstractListModel interface
    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Set the recordings to display
    void setRecordings(std::vector<recording_info> recordings);

    /// Get recordings
    auto recordings() const -> std::vector<recording_info> const&;

    /// Get recording at index
    auto recordingAt(int index) const -> recording_info const*;

    /// Find index by file hash
    auto indexForHash(std::string const& hash) const -> int;

    /// Update a single recording
    void updateRecording(int index, recording_info const& info);

    /// Remove recording at index
    void removeRecording(int index);

    /// Add a recording
    void addRecording(recording_info const& info);

    /// Clear all recordings
    void clear();

public slots:
    /// Update tags at index
    void updateTags(int index, QStringList const& tags);

    /// Update notes at index
    void updateNotes(int index, QString const& notes);

    /// Update rating at index (1-5, 0 = unrated)
    void updateRating(int index, int rating);

    /// Update export status at index
    void updateExportStatus(int index, int status);

signals:
    void recordingUpdated(int index);
    void recordingRemoved(QString const& fileHash);
    void recordingAdded(int index);

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::file_manager
