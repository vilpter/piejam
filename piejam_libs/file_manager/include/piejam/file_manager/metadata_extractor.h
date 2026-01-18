// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/file_manager/recording.h>

#include <functional>
#include <string>

namespace piejam::file_manager
{

/// Extracts metadata from audio files using libsndfile
class metadata_extractor
{
public:
    metadata_extractor();
    ~metadata_extractor();

    metadata_extractor(metadata_extractor const&) = delete;
    metadata_extractor& operator=(metadata_extractor const&) = delete;
    metadata_extractor(metadata_extractor&&) noexcept;
    metadata_extractor& operator=(metadata_extractor&&) noexcept;

    /// Extract metadata from an audio file
    /// Returns empty metadata with file_path set if extraction fails
    auto extract(std::string const& file_path) -> recording_metadata;

    /// Calculate SHA256 hash of a file
    /// Returns empty string on error
    auto calculate_hash(std::string const& file_path) -> std::string;

    /// Analyze audio levels (peak, RMS, clipping)
    /// Updates the provided metadata struct
    void analyze_levels(std::string const& file_path, recording_metadata& metadata);

    /// Get format from file extension
    static auto format_from_extension(std::string const& path) -> audio_format;

    /// Check if file is valid audio file that can be read
    auto is_valid_audio_file(std::string const& file_path) -> bool;

    /// Get last error message
    auto last_error() const -> std::string;

private:
    struct impl;
    std::unique_ptr<impl> m_impl;
};

} // namespace piejam::file_manager
