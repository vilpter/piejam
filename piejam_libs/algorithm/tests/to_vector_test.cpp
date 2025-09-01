// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/algorithm/to_vector.h>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <list>
#include <ranges>
#include <string>

namespace piejam::algorithm::test
{

TEST(to_vector, works_with_vector)
{
    std::vector<int> input{1, 2, 3, 4};
    auto result = to_vector(input);

    ASSERT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 4);
}

TEST(to_vector, works_with_list)
{
    std::list<std::string> input{"a", "bb", "ccc"};
    auto result = to_vector(input);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "bb");
    EXPECT_EQ(result[2], "ccc");
}

TEST(to_vector, works_with_input_range_stream)
{
    std::istringstream iss{"10 20 30"};
    auto input = std::ranges::istream_view<int>(iss);

    auto result = to_vector(input);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], 10);
    EXPECT_EQ(result[1], 20);
    EXPECT_EQ(result[2], 30);
}

TEST(to_vector, works_with_empty_range)
{
    std::vector<int> input{};
    auto result = to_vector(input);

    EXPECT_TRUE(result.empty());
}

TEST(to_vector, works_with_move_only_input_range)
{
    std::vector<std::unique_ptr<int>> input;
    input.push_back(std::make_unique<int>(1));
    input.push_back(std::make_unique<int>(2));
    input.push_back(std::make_unique<int>(3));

    auto result = to_vector(input);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(*result[0], 1);
    EXPECT_EQ(*result[1], 2);
    EXPECT_EQ(*result[2], 3);

    // original input elements have been moved-from
    EXPECT_EQ(input[0], nullptr);
    EXPECT_EQ(input[1], nullptr);
    EXPECT_EQ(input[2], nullptr);
}

TEST(to_vector, works_with_pipeline_transform)
{
    std::vector<int> input{1, 2, 3};
    auto result = input | std::views::transform([](int x) { return x * 2; }) |
                  to_vector;

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[1], 4);
    EXPECT_EQ(result[2], 6);
}

} // namespace piejam::algorithm::test
