// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording_scanner.h>

#include <gtest/gtest.h>

namespace piejam::file_manager::test
{

TEST(recording_scanner_test, is_audio_file_wav)
{
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.wav"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.WAV"));
}

TEST(recording_scanner_test, is_audio_file_flac)
{
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.flac"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.FLAC"));
}

TEST(recording_scanner_test, is_audio_file_aiff)
{
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.aiff"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.AIFF"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.aif"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.AIF"));
}

TEST(recording_scanner_test, is_audio_file_ogg)
{
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.ogg"));
    EXPECT_TRUE(recording_scanner::is_audio_file("/path/to/file.OGG"));
}

TEST(recording_scanner_test, is_audio_file_non_audio)
{
    EXPECT_FALSE(recording_scanner::is_audio_file("/path/to/file.txt"));
    EXPECT_FALSE(recording_scanner::is_audio_file("/path/to/file.mp3"));
    EXPECT_FALSE(recording_scanner::is_audio_file("/path/to/file.jpg"));
    EXPECT_FALSE(recording_scanner::is_audio_file("/path/to/file"));
}

TEST(recording_scanner_test, supported_extensions)
{
    auto exts = recording_scanner::supported_extensions();

    EXPECT_EQ(exts.size(), 5u);
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".wav"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".flac"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".aiff"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".aif"), exts.end());
    EXPECT_NE(std::find(exts.begin(), exts.end(), ".ogg"), exts.end());
}

TEST(recording_scanner_test, add_scan_directory)
{
    recording_scanner scanner;

    scanner.add_scan_directory("/test/path1");
    scanner.add_scan_directory("/test/path2");

    auto dirs = scanner.scan_directories();
    EXPECT_EQ(dirs.size(), 2u);
}

TEST(recording_scanner_test, add_duplicate_directory)
{
    recording_scanner scanner;

    scanner.add_scan_directory("/test/path");
    scanner.add_scan_directory("/test/path"); // Duplicate

    auto dirs = scanner.scan_directories();
    EXPECT_EQ(dirs.size(), 1u);
}

TEST(recording_scanner_test, remove_scan_directory)
{
    recording_scanner scanner;

    scanner.add_scan_directory("/test/path1");
    scanner.add_scan_directory("/test/path2");
    scanner.remove_scan_directory("/test/path1");

    auto dirs = scanner.scan_directories();
    EXPECT_EQ(dirs.size(), 1u);
    EXPECT_EQ(dirs[0], "/test/path2");
}

TEST(recording_scanner_test, auto_scan_default_disabled)
{
    recording_scanner scanner;
    EXPECT_FALSE(scanner.auto_scan_enabled());
}

TEST(recording_scanner_test, set_auto_scan)
{
    recording_scanner scanner;

    scanner.set_auto_scan(true);
    EXPECT_TRUE(scanner.auto_scan_enabled());

    scanner.set_auto_scan(false);
    EXPECT_FALSE(scanner.auto_scan_enabled());
}

} // namespace piejam::file_manager::test
