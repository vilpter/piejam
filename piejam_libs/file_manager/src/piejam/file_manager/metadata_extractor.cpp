// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/metadata_extractor.h>

#include <sndfile.h>

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

#include <algorithm>
#include <cmath>

namespace piejam::file_manager
{

struct metadata_extractor::impl
{
    std::string last_error;
};

metadata_extractor::metadata_extractor()
    : m_impl(std::make_unique<impl>())
{
}

metadata_extractor::~metadata_extractor() = default;

metadata_extractor::metadata_extractor(metadata_extractor&&) noexcept = default;
metadata_extractor& metadata_extractor::operator=(metadata_extractor&&) noexcept = default;

auto
metadata_extractor::extract(std::string const& file_path) -> recording_metadata
{
    recording_metadata metadata;
    metadata.file_path = file_path;

    QFileInfo file_info(QString::fromStdString(file_path));
    metadata.filename = file_info.fileName().toStdString();
    metadata.file_size = static_cast<uint64_t>(file_info.size());
    metadata.format = format_from_extension(file_path);
    metadata.storage_location = "local";

    // Get file timestamps
    metadata.last_modified = std::chrono::system_clock::from_time_t(
        file_info.lastModified().toSecsSinceEpoch());
    metadata.recorded_at = std::chrono::system_clock::from_time_t(
        file_info.birthTime().isValid()
            ? file_info.birthTime().toSecsSinceEpoch()
            : file_info.lastModified().toSecsSinceEpoch());

    // Open with libsndfile
    SF_INFO sfinfo;
    sfinfo.format = 0;

    SNDFILE* sndfile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        m_impl->last_error = sf_strerror(nullptr);
        return metadata;
    }

    // Extract audio info
    metadata.sample_rate = static_cast<uint32_t>(sfinfo.samplerate);
    metadata.channels = static_cast<uint16_t>(sfinfo.channels);

    // Calculate duration
    if (sfinfo.samplerate > 0)
    {
        metadata.duration_ms = static_cast<uint64_t>(
            (sfinfo.frames * 1000) / sfinfo.samplerate);
    }

    // Determine bit depth from format
    int sub_format = sfinfo.format & SF_FORMAT_SUBMASK;
    switch (sub_format)
    {
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
        metadata.bit_depth = 8;
        break;
    case SF_FORMAT_PCM_16:
        metadata.bit_depth = 16;
        break;
    case SF_FORMAT_PCM_24:
        metadata.bit_depth = 24;
        break;
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_FLOAT:
        metadata.bit_depth = 32;
        break;
    case SF_FORMAT_DOUBLE:
        metadata.bit_depth = 64;
        break;
    default:
        metadata.bit_depth = 16; // Default assumption
        break;
    }

    // Determine format from major format
    int major_format = sfinfo.format & SF_FORMAT_TYPEMASK;
    switch (major_format)
    {
    case SF_FORMAT_WAV:
    case SF_FORMAT_WAVEX:
        metadata.format = audio_format::wav;
        break;
    case SF_FORMAT_FLAC:
        metadata.format = audio_format::flac;
        break;
    case SF_FORMAT_AIFF:
        metadata.format = audio_format::aiff;
        break;
    case SF_FORMAT_OGG:
        metadata.format = audio_format::ogg;
        break;
    default:
        break;
    }

    sf_close(sndfile);

    // Calculate hash (can be slow for large files, so we do a partial hash)
    metadata.file_hash = calculate_hash(file_path);

    m_impl->last_error.clear();
    return metadata;
}

auto
metadata_extractor::calculate_hash(std::string const& file_path) -> std::string
{
    QFile file(QString::fromStdString(file_path));
    if (!file.open(QIODevice::ReadOnly))
    {
        m_impl->last_error = "Cannot open file for hashing";
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    // For large files, we use a sampling approach for performance:
    // - First 1MB
    // - Middle 1MB
    // - Last 1MB
    // - File size as part of hash
    static constexpr qint64 sample_size = 1024 * 1024; // 1MB

    qint64 file_size = file.size();

    if (file_size <= sample_size * 3)
    {
        // Small file - hash entire content
        if (!hash.addData(&file))
        {
            m_impl->last_error = "Error reading file for hashing";
            return {};
        }
    }
    else
    {
        // Large file - sample-based hashing
        QByteArray buffer;

        // First 1MB
        buffer = file.read(sample_size);
        hash.addData(buffer);

        // Middle 1MB
        file.seek(file_size / 2 - sample_size / 2);
        buffer = file.read(sample_size);
        hash.addData(buffer);

        // Last 1MB
        file.seek(file_size - sample_size);
        buffer = file.read(sample_size);
        hash.addData(buffer);

        // Include file size in hash
        QByteArray size_bytes;
        size_bytes.setNum(file_size);
        hash.addData(size_bytes);
    }

    return hash.result().toHex().toStdString();
}

void
metadata_extractor::analyze_levels(std::string const& file_path,
                                   recording_metadata& metadata)
{
    SF_INFO sfinfo;
    sfinfo.format = 0;

    SNDFILE* sndfile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        m_impl->last_error = sf_strerror(nullptr);
        return;
    }

    static constexpr sf_count_t buffer_size = 8192;
    std::vector<float> buffer(static_cast<size_t>(buffer_size * sfinfo.channels));

    float peak_level = 0.0f;
    double sum_squares = 0.0;
    sf_count_t total_samples = 0;
    int clip_count = 0;

    static constexpr float clip_threshold = 0.99f;

    sf_count_t read_count;
    while ((read_count = sf_readf_float(sndfile, buffer.data(), buffer_size)) > 0)
    {
        sf_count_t samples_read = read_count * sfinfo.channels;

        for (sf_count_t i = 0; i < samples_read; ++i)
        {
            float sample = std::abs(buffer[static_cast<size_t>(i)]);

            if (sample > peak_level)
            {
                peak_level = sample;
            }

            sum_squares += static_cast<double>(sample) * sample;

            if (sample >= clip_threshold)
            {
                ++clip_count;
            }
        }

        total_samples += samples_read;
    }

    sf_close(sndfile);

    // Calculate peak level in dB
    if (peak_level > 0.0f)
    {
        metadata.peak_level_db = 20.0f * std::log10(peak_level);
    }
    else
    {
        metadata.peak_level_db = -100.0f;
    }

    // Calculate RMS level in dB
    if (total_samples > 0)
    {
        double rms = std::sqrt(sum_squares / static_cast<double>(total_samples));
        if (rms > 0.0)
        {
            metadata.rms_level_db = static_cast<float>(20.0 * std::log10(rms));
        }
        else
        {
            metadata.rms_level_db = -100.0f;
        }
    }

    metadata.has_clipping = clip_count > 0;
    metadata.clip_count = clip_count;
}

auto
metadata_extractor::format_from_extension(std::string const& path) -> audio_format
{
    auto dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos)
    {
        return audio_format::unknown;
    }

    std::string ext = path.substr(dot_pos);

    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (ext == ".wav")
    {
        return audio_format::wav;
    }
    if (ext == ".flac")
    {
        return audio_format::flac;
    }
    if (ext == ".aiff" || ext == ".aif")
    {
        return audio_format::aiff;
    }
    if (ext == ".ogg")
    {
        return audio_format::ogg;
    }

    return audio_format::unknown;
}

auto
metadata_extractor::is_valid_audio_file(std::string const& file_path) -> bool
{
    SF_INFO sfinfo;
    sfinfo.format = 0;

    SNDFILE* sndfile = sf_open(file_path.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        m_impl->last_error = sf_strerror(nullptr);
        return false;
    }

    sf_close(sndfile);
    return true;
}

auto
metadata_extractor::last_error() const -> std::string
{
    return m_impl->last_error;
}

} // namespace piejam::file_manager
