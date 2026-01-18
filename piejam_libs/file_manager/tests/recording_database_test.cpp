// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording_database.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace piejam::file_manager::test
{

class recording_database_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_db_path = std::filesystem::temp_directory_path() /
                    "piejam_test_recordings.json";
        std::filesystem::remove(m_db_path);
    }

    void TearDown() override
    {
        std::filesystem::remove(m_db_path);
    }

    std::filesystem::path m_db_path;
};

TEST_F(recording_database_test, constructor_creates_empty_database)
{
    recording_database db(m_db_path.string());

    EXPECT_EQ(db.recording_count(), 0u);
}

TEST_F(recording_database_test, set_and_get_recording_metadata)
{
    recording_database db(m_db_path.string());

    recording_metadata meta;
    meta.file_hash = "abc123";
    meta.file_path = "/test/file.wav";
    meta.filename = "file.wav";
    meta.sample_rate = 48000;
    meta.channels = 2;

    db.set_recording_metadata(meta);

    auto retrieved = db.get_recording_metadata("abc123");
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->file_hash, "abc123");
    EXPECT_EQ(retrieved->file_path, "/test/file.wav");
    EXPECT_EQ(retrieved->filename, "file.wav");
    EXPECT_EQ(retrieved->sample_rate, 48000u);
    EXPECT_EQ(retrieved->channels, 2);
}

TEST_F(recording_database_test, get_nonexistent_metadata_returns_nullopt)
{
    recording_database db(m_db_path.string());

    auto retrieved = db.get_recording_metadata("nonexistent");
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(recording_database_test, remove_recording_metadata)
{
    recording_database db(m_db_path.string());

    recording_metadata meta;
    meta.file_hash = "abc123";

    db.set_recording_metadata(meta);
    EXPECT_EQ(db.recording_count(), 1u);

    bool removed = db.remove_recording_metadata("abc123");
    EXPECT_TRUE(removed);
    EXPECT_EQ(db.recording_count(), 0u);
}

TEST_F(recording_database_test, remove_nonexistent_returns_false)
{
    recording_database db(m_db_path.string());

    bool removed = db.remove_recording_metadata("nonexistent");
    EXPECT_FALSE(removed);
}

TEST_F(recording_database_test, update_tags)
{
    recording_database db(m_db_path.string());

    db.update_tags("hash1", {"tag1", "tag2"});

    auto user = db.get_user_metadata("hash1");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->tags.size(), 2u);
    EXPECT_EQ(user->tags[0], "tag1");
    EXPECT_EQ(user->tags[1], "tag2");
}

TEST_F(recording_database_test, all_tags_returns_unique_tags)
{
    recording_database db(m_db_path.string());

    db.update_tags("hash1", {"tag1", "tag2"});
    db.update_tags("hash2", {"tag2", "tag3"});

    auto all_tags = db.all_tags();
    EXPECT_EQ(all_tags.size(), 3u);
}

TEST_F(recording_database_test, recordings_with_tag)
{
    recording_database db(m_db_path.string());

    db.update_tags("hash1", {"jazz", "live"});
    db.update_tags("hash2", {"rock", "live"});
    db.update_tags("hash3", {"jazz"});

    auto jazz_recordings = db.recordings_with_tag("jazz");
    EXPECT_EQ(jazz_recordings.size(), 2u);

    auto live_recordings = db.recordings_with_tag("live");
    EXPECT_EQ(live_recordings.size(), 2u);

    auto rock_recordings = db.recordings_with_tag("rock");
    EXPECT_EQ(rock_recordings.size(), 1u);
}

TEST_F(recording_database_test, update_notes)
{
    recording_database db(m_db_path.string());

    db.update_notes("hash1", "Great recording!");

    auto user = db.get_user_metadata("hash1");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->notes, "Great recording!");
}

TEST_F(recording_database_test, update_rating)
{
    recording_database db(m_db_path.string());

    db.update_rating("hash1", 4);

    auto user = db.get_user_metadata("hash1");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->rating, 4);
}

TEST_F(recording_database_test, update_rating_clamps_values)
{
    recording_database db(m_db_path.string());

    db.update_rating("hash1", 10);
    auto user1 = db.get_user_metadata("hash1");
    ASSERT_TRUE(user1.has_value());
    EXPECT_EQ(user1->rating, 5);

    db.update_rating("hash2", -5);
    auto user2 = db.get_user_metadata("hash2");
    ASSERT_TRUE(user2.has_value());
    EXPECT_EQ(user2->rating, 0);
}

TEST_F(recording_database_test, recordings_with_min_rating)
{
    recording_database db(m_db_path.string());

    db.update_rating("hash1", 5);
    db.update_rating("hash2", 4);
    db.update_rating("hash3", 3);
    db.update_rating("hash4", 2);

    auto high_rated = db.recordings_with_min_rating(4);
    EXPECT_EQ(high_rated.size(), 2u);
}

TEST_F(recording_database_test, update_export_status)
{
    recording_database db(m_db_path.string());

    db.update_export_status("hash1", export_status::exported);

    auto user = db.get_user_metadata("hash1");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->status, export_status::exported);
}

TEST_F(recording_database_test, recordings_with_status)
{
    recording_database db(m_db_path.string());

    db.update_export_status("hash1", export_status::new_recording);
    db.update_export_status("hash2", export_status::exported);
    db.update_export_status("hash3", export_status::new_recording);

    auto new_recordings = db.recordings_with_status(export_status::new_recording);
    EXPECT_EQ(new_recordings.size(), 2u);

    auto exported = db.recordings_with_status(export_status::exported);
    EXPECT_EQ(exported.size(), 1u);
}

TEST_F(recording_database_test, save_and_load)
{
    {
        recording_database db(m_db_path.string());

        recording_metadata meta;
        meta.file_hash = "persistent_hash";
        meta.file_path = "/test/file.wav";
        meta.filename = "file.wav";
        meta.sample_rate = 96000;

        db.set_recording_metadata(meta);
        db.update_tags("persistent_hash", {"saved_tag"});
        db.update_rating("persistent_hash", 5);
        db.save();
    }

    // Reload from file
    recording_database db2(m_db_path.string());

    auto meta = db2.get_recording_metadata("persistent_hash");
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->file_path, "/test/file.wav");
    EXPECT_EQ(meta->sample_rate, 96000u);

    auto user = db2.get_user_metadata("persistent_hash");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->tags.size(), 1u);
    EXPECT_EQ(user->tags[0], "saved_tag");
    EXPECT_EQ(user->rating, 5);
}

TEST_F(recording_database_test, prune_deleted_files)
{
    recording_database db(m_db_path.string());

    recording_metadata meta1;
    meta1.file_hash = "hash1";
    db.set_recording_metadata(meta1);

    recording_metadata meta2;
    meta2.file_hash = "hash2";
    db.set_recording_metadata(meta2);

    recording_metadata meta3;
    meta3.file_hash = "hash3";
    db.set_recording_metadata(meta3);

    db.update_tags("hash1", {"tag1"});
    db.update_tags("hash2", {"tag2"});

    EXPECT_EQ(db.recording_count(), 3u);

    // Only hash1 and hash3 are valid
    db.prune_deleted_files({"hash1", "hash3"});

    EXPECT_EQ(db.recording_count(), 2u);
    EXPECT_TRUE(db.get_recording_metadata("hash1").has_value());
    EXPECT_FALSE(db.get_recording_metadata("hash2").has_value());
    EXPECT_TRUE(db.get_recording_metadata("hash3").has_value());

    // User data should also be pruned
    EXPECT_TRUE(db.get_user_metadata("hash1").has_value());
    EXPECT_FALSE(db.get_user_metadata("hash2").has_value());
}

TEST_F(recording_database_test, get_recording_info_combines_data)
{
    recording_database db(m_db_path.string());

    recording_metadata meta;
    meta.file_hash = "hash1";
    meta.filename = "test.wav";
    meta.sample_rate = 48000;
    db.set_recording_metadata(meta);

    db.update_tags("hash1", {"tag1"});
    db.update_rating("hash1", 4);

    auto info = db.get_recording_info("hash1");
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->metadata.filename, "test.wav");
    EXPECT_EQ(info->metadata.sample_rate, 48000u);
    EXPECT_EQ(info->user_data.tags.size(), 1u);
    EXPECT_EQ(info->user_data.rating, 4);
}

} // namespace piejam::file_manager::test
