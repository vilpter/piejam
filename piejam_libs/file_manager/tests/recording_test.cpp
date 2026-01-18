// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording.h>

#include <gtest/gtest.h>

namespace piejam::file_manager::test
{

TEST(recording_test, audio_format_to_string_wav)
{
    EXPECT_STREQ(audio_format_to_string(audio_format::wav), "WAV");
}

TEST(recording_test, audio_format_to_string_flac)
{
    EXPECT_STREQ(audio_format_to_string(audio_format::flac), "FLAC");
}

TEST(recording_test, audio_format_to_string_aiff)
{
    EXPECT_STREQ(audio_format_to_string(audio_format::aiff), "AIFF");
}

TEST(recording_test, audio_format_to_string_ogg)
{
    EXPECT_STREQ(audio_format_to_string(audio_format::ogg), "OGG");
}

TEST(recording_test, audio_format_to_string_unknown)
{
    EXPECT_STREQ(audio_format_to_string(audio_format::unknown), "Unknown");
}

TEST(recording_test, export_status_to_string_new)
{
    EXPECT_STREQ(export_status_to_string(export_status::new_recording), "New");
}

TEST(recording_test, export_status_to_string_archived)
{
    EXPECT_STREQ(export_status_to_string(export_status::archived), "Archived");
}

TEST(recording_test, export_status_to_string_exported)
{
    EXPECT_STREQ(export_status_to_string(export_status::exported), "Exported");
}

TEST(recording_test, export_status_to_string_needs_processing)
{
    EXPECT_STREQ(export_status_to_string(export_status::needs_processing), "Needs Processing");
}

TEST(recording_test, recording_metadata_default_values)
{
    recording_metadata meta;

    EXPECT_TRUE(meta.file_path.empty());
    EXPECT_TRUE(meta.filename.empty());
    EXPECT_TRUE(meta.file_hash.empty());
    EXPECT_EQ(meta.file_size, 0u);
    EXPECT_EQ(meta.duration_ms, 0u);
    EXPECT_EQ(meta.sample_rate, 0u);
    EXPECT_EQ(meta.bit_depth, 0);
    EXPECT_EQ(meta.channels, 0);
    EXPECT_EQ(meta.format, audio_format::unknown);
    EXPECT_FLOAT_EQ(meta.peak_level_db, -100.0f);
    EXPECT_FLOAT_EQ(meta.rms_level_db, -100.0f);
    EXPECT_FALSE(meta.has_clipping);
    EXPECT_EQ(meta.clip_count, 0);
}

TEST(recording_test, user_metadata_default_values)
{
    user_metadata user;

    EXPECT_TRUE(user.file_hash.empty());
    EXPECT_TRUE(user.tags.empty());
    EXPECT_TRUE(user.notes.empty());
    EXPECT_EQ(user.rating, 0);
    EXPECT_EQ(user.status, export_status::new_recording);
    EXPECT_TRUE(user.project_name.empty());
    EXPECT_TRUE(user.artist_names.empty());
    EXPECT_TRUE(user.location_venue.empty());
}

TEST(recording_test, recording_info_duration_string_seconds)
{
    recording_info info;
    info.metadata.duration_ms = 45000; // 45 seconds

    EXPECT_EQ(info.duration_string(), "0:45");
}

TEST(recording_test, recording_info_duration_string_minutes)
{
    recording_info info;
    info.metadata.duration_ms = 185000; // 3:05

    EXPECT_EQ(info.duration_string(), "3:05");
}

TEST(recording_test, recording_info_duration_string_hours)
{
    recording_info info;
    info.metadata.duration_ms = 3725000; // 1:02:05

    EXPECT_EQ(info.duration_string(), "1:02:05");
}

TEST(recording_test, recording_info_file_size_string_bytes)
{
    recording_info info;
    info.metadata.file_size = 512;

    EXPECT_EQ(info.file_size_string(), "512 B");
}

TEST(recording_test, recording_info_file_size_string_kb)
{
    recording_info info;
    info.metadata.file_size = 2048;

    EXPECT_EQ(info.file_size_string(), "2.0 KB");
}

TEST(recording_test, recording_info_file_size_string_mb)
{
    recording_info info;
    info.metadata.file_size = 5242880; // 5 MB

    EXPECT_EQ(info.file_size_string(), "5.0 MB");
}

TEST(recording_test, recording_info_file_size_string_gb)
{
    recording_info info;
    info.metadata.file_size = 2147483648; // 2 GB

    EXPECT_EQ(info.file_size_string(), "2.0 GB");
}

} // namespace piejam::file_manager::test
