// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/file_manager/recording.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace piejam::file_manager
{

/// JSON-based database for recording metadata and user data
/// Storage location: ~/.piejam/recordings_metadata.json
class recording_database
{
public:
    explicit recording_database(std::string const& db_path);
    ~recording_database();

    recording_database(recording_database const&) = delete;
    recording_database& operator=(recording_database const&) = delete;
    recording_database(recording_database&&) noexcept;
    recording_database& operator=(recording_database&&) noexcept;

    /// Load database from file
    auto load() -> bool;

    /// Save database to file
    auto save() -> bool;

    /// Check if database file exists
    auto exists() const -> bool;

    /// Get database file path
    auto path() const -> std::string;

    // Recording metadata operations

    /// Add or update recording metadata
    void set_recording_metadata(recording_metadata const& metadata);

    /// Get recording metadata by file hash
    auto get_recording_metadata(std::string const& file_hash)
        -> std::optional<recording_metadata>;

    /// Get recording metadata by file path
    auto get_recording_metadata_by_path(std::string const& file_path)
        -> std::optional<recording_metadata>;

    /// Remove recording metadata
    auto remove_recording_metadata(std::string const& file_hash) -> bool;

    /// Get all recording metadata
    auto all_recording_metadata() -> std::vector<recording_metadata>;

    // User metadata operations

    /// Add or update user metadata
    void set_user_metadata(user_metadata const& metadata);

    /// Get user metadata by file hash
    auto get_user_metadata(std::string const& file_hash)
        -> std::optional<user_metadata>;

    /// Remove user metadata
    auto remove_user_metadata(std::string const& file_hash) -> bool;

    /// Get all user metadata
    auto all_user_metadata() -> std::vector<user_metadata>;

    // Combined operations

    /// Get full recording info (metadata + user data)
    auto get_recording_info(std::string const& file_hash)
        -> std::optional<recording_info>;

    /// Get all recordings with combined info
    auto all_recordings() -> std::vector<recording_info>;

    // Tag operations

    /// Update tags for a recording
    void update_tags(std::string const& file_hash, std::vector<std::string> const& tags);

    /// Get all unique tags used
    auto all_tags() -> std::vector<std::string>;

    /// Get recordings with a specific tag
    auto recordings_with_tag(std::string const& tag) -> std::vector<std::string>;

    // Notes operations

    /// Update notes for a recording
    void update_notes(std::string const& file_hash, std::string const& notes);

    // Rating operations

    /// Update rating for a recording (1-5, 0 = unrated)
    void update_rating(std::string const& file_hash, int rating);

    /// Get recordings with minimum rating
    auto recordings_with_min_rating(int min_rating) -> std::vector<std::string>;

    // Export status operations

    /// Update export status for a recording
    void update_export_status(std::string const& file_hash, export_status status);

    /// Get recordings with specific export status
    auto recordings_with_status(export_status status) -> std::vector<std::string>;

    // Cleanup operations

    /// Remove entries for files that no longer exist
    /// Pass list of valid file hashes, removes any not in the list
    void prune_deleted_files(std::vector<std::string> const& valid_hashes);

    /// Get count of recordings in database
    auto recording_count() const -> size_t;

    /// Get last error message
    auto last_error() const -> std::string;

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::file_manager
