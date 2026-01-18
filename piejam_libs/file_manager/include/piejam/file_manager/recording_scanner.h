// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/file_manager/fwd.h>

#include <QObject>
#include <QFileSystemWatcher>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace piejam::file_manager
{

/// Scans directories for audio recording files
class recording_scanner : public QObject
{
    Q_OBJECT

public:
    using new_file_callback = std::function<void(std::string const&)>;
    using file_removed_callback = std::function<void(std::string const&)>;
    using scan_complete_callback = std::function<void(std::vector<std::string> const&)>;

    explicit recording_scanner(QObject* parent = nullptr);
    ~recording_scanner() override;

    recording_scanner(recording_scanner const&) = delete;
    recording_scanner& operator=(recording_scanner const&) = delete;
    recording_scanner(recording_scanner&&) noexcept;
    recording_scanner& operator=(recording_scanner&&) noexcept;

    /// Add a directory to scan
    void add_scan_directory(std::string const& path);

    /// Remove a directory from scanning
    void remove_scan_directory(std::string const& path);

    /// Get list of scan directories
    auto scan_directories() const -> std::vector<std::string>;

    /// Perform a full scan of all directories
    /// Returns list of all audio file paths found
    auto scan() -> std::vector<std::string>;

    /// Perform async scan with callback
    void scan_async(scan_complete_callback callback);

    /// Enable/disable file system watching for new files
    void set_auto_scan(bool enable);

    /// Check if auto scan is enabled
    auto auto_scan_enabled() const -> bool;

    /// Check if a file extension is a supported audio format
    static auto is_audio_file(std::string const& path) -> bool;

    /// Set callback for new files detected
    void set_new_file_callback(new_file_callback callback);

    /// Set callback for removed files detected
    void set_file_removed_callback(file_removed_callback callback);

    /// Get supported file extensions
    static auto supported_extensions() -> std::vector<std::string>;

signals:
    void scanStarted();
    void scanComplete(int fileCount);
    void newFileDetected(QString const& filePath);
    void fileRemoved(QString const& filePath);
    void scanProgress(int current, int total);

private slots:
    void onDirectoryChanged(QString const& path);
    void onFileChanged(QString const& path);

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::file_manager
