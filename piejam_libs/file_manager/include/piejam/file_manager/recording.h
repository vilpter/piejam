// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace piejam::file_manager
{

/// Audio format/codec type
enum class audio_format
{
    wav,
    flac,
    aiff,
    ogg,
    unknown
};

/// Export status for recordings
enum class export_status
{
    new_recording,
    archived,
    exported,
    needs_processing
};

/// Convert audio_format to display string
inline auto
audio_format_to_string(audio_format f) noexcept -> char const*
{
    switch (f)
    {
    case audio_format::wav:
        return "WAV";
    case audio_format::flac:
        return "FLAC";
    case audio_format::aiff:
        return "AIFF";
    case audio_format::ogg:
        return "OGG";
    default:
        return "Unknown";
    }
}

/// Convert export_status to display string
inline auto
export_status_to_string(export_status s) noexcept -> char const*
{
    switch (s)
    {
    case export_status::new_recording:
        return "New";
    case export_status::archived:
        return "Archived";
    case export_status::exported:
        return "Exported";
    case export_status::needs_processing:
        return "Needs Processing";
    default:
        return "Unknown";
    }
}

/// Audio metadata extracted from file
struct recording_metadata
{
    std::string file_path;      ///< Full path to file
    std::string filename;       ///< Filename only
    std::string file_hash;      ///< SHA256 hash for integrity/identification
    uint64_t file_size{0};      ///< File size in bytes
    uint64_t duration_ms{0};    ///< Duration in milliseconds
    uint32_t sample_rate{0};    ///< Sample rate in Hz
    uint16_t bit_depth{0};      ///< Bit depth (16, 24, 32)
    uint16_t channels{0};       ///< Number of channels
    audio_format format{audio_format::unknown};

    // Audio analysis
    float peak_level_db{-100.0f};  ///< Peak level in dB
    float rms_level_db{-100.0f};   ///< RMS level in dB
    bool has_clipping{false};      ///< Whether audio clipped
    int clip_count{0};             ///< Number of clipped samples

    // Timestamps
    std::chrono::system_clock::time_point recorded_at;
    std::chrono::system_clock::time_point last_modified;

    // Storage location
    std::string storage_location;  ///< "local" or "nfs:<mount_id>"
};

/// User-editable metadata stored in database
struct user_metadata
{
    std::string file_hash;         ///< Links to recording_metadata
    std::vector<std::string> tags; ///< User tags (e.g., ["rehearsal", "jazz"])
    std::string notes;             ///< User notes/description
    int rating{0};                 ///< 1-5 stars, 0 = unrated
    export_status status{export_status::new_recording};
    std::string project_name;      ///< Project/album association
    std::string artist_names;      ///< Performer credits
    std::string location_venue;    ///< Recording location

    // Timestamps
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point modified_at;
};

/// Combined recording info for display
struct recording_info
{
    recording_metadata metadata;
    user_metadata user_data;

    /// Get display-friendly duration string (MM:SS or HH:MM:SS)
    auto duration_string() const -> std::string
    {
        uint64_t total_seconds = metadata.duration_ms / 1000;
        uint64_t hours = total_seconds / 3600;
        uint64_t minutes = (total_seconds % 3600) / 60;
        uint64_t seconds = total_seconds % 60;

        char buf[32];
        if (hours > 0)
        {
            snprintf(buf, sizeof(buf), "%llu:%02llu:%02llu",
                     static_cast<unsigned long long>(hours),
                     static_cast<unsigned long long>(minutes),
                     static_cast<unsigned long long>(seconds));
        }
        else
        {
            snprintf(buf, sizeof(buf), "%llu:%02llu",
                     static_cast<unsigned long long>(minutes),
                     static_cast<unsigned long long>(seconds));
        }
        return buf;
    }

    /// Get display-friendly file size string (KB, MB, GB)
    auto file_size_string() const -> std::string
    {
        uint64_t size = metadata.file_size;
        char buf[32];

        if (size >= 1073741824) // GB
        {
            snprintf(buf, sizeof(buf), "%.1f GB", size / 1073741824.0);
        }
        else if (size >= 1048576) // MB
        {
            snprintf(buf, sizeof(buf), "%.1f MB", size / 1048576.0);
        }
        else if (size >= 1024) // KB
        {
            snprintf(buf, sizeof(buf), "%.1f KB", size / 1024.0);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%llu B", static_cast<unsigned long long>(size));
        }
        return buf;
    }
};

} // namespace piejam::file_manager
