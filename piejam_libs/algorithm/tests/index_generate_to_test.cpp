// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/algorithm/index_generate_to.h>

#include <gtest/gtest.h>

#include <list>
#include <ranges>
#include <vector>

namespace piejam::algorithm
{

TEST(index_generator, concept_accepts_valid_function)
{
    auto func = [](std::size_t i) { return i * 2; };
    static_assert(index_generator<decltype(func)>);
}

TEST(index_generator, concept_rejects_function_which_returns_void)
{
    auto func = [](std::size_t) {};
    static_assert(!index_generator<decltype(func)>);
}

TEST(index_generator, concept_rejects_non_function_type)
{
    static_assert(!index_generator<int>);
}

TEST(index_generator, concept_rejects_function_with_wrong_parameter_type)
{
    auto func = [](std::string i) { return i; };
    static_assert(!index_generator<decltype(func)>);
}

TEST(index_generator_for_range, concept_accepts_valid_function_and_range)
{
    auto func = [](std::size_t i) { return i * 2; };
    static_assert(index_generator_for_range<decltype(func), std::vector<int>>);
}

TEST(index_generator_for_range, concept_rejects_function_with_wrong_return_type)
{
    auto func = [](std::size_t) { return std::string{}; };
    static_assert(!index_generator_for_range<decltype(func), std::vector<int>>);
}

TEST(index_generator_for_range, concept_rejects_non_function_type)
{
    static_assert(!index_generator_for_range<int, std::vector<int>>);
}

TEST(index_generator_for_range,
     concept_rejects_function_with_wrong_parameter_type)
{
    auto func = [](std::string i) { return i; };
    static_assert(!index_generator_for_range<decltype(func), std::vector<int>>);
}

TEST(index_generator_view, produces_correct_values)
{
    auto view = index_generator_view([](std::size_t i) { return i * i; }) |
                std::views::take(5);
    std::vector<int> expected{0, 1, 4, 9, 16};

    auto it = std::ranges::begin(view);
    for (auto e : expected)
    {
        ASSERT_EQ(*it, e);
        ++it;
    }
}

TEST(index_generator_view, empty_count_returns_empty_view)
{
    auto view = index_generator_view([](std::size_t i) { return i * i; }) |
                std::views::take(0);
    ASSERT_TRUE(view.begin() == view.end());
}

TEST(index_generate_to, fills_vector_with_indexed_values)
{
    std::vector<int> v(5);
    index_generate_to(v, [](std::size_t i) { return i * 10; });

    std::vector<int> expected{0, 10, 20, 30, 40};
    ASSERT_EQ(v, expected);
}

TEST(index_generate_to, works_with_floating_point)
{
    std::vector<double> v(4);
    index_generate_to(v, [](std::size_t i) { return i * 0.5; });

    std::vector<double> expected{0.0, 0.5, 1.0, 1.5};
    for (size_t i = 0; i < v.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(v[i], expected[i]);
    }
}

TEST(index_generate_to, empty_range_remains_empty)
{
    std::vector<int> v;
    index_generate_to(v, [](std::size_t i) { return i; });
    ASSERT_TRUE(v.empty());
}

TEST(index_generate_to, works_with_non_vector_ranges)
{
    std::list<int> lst(3);
    index_generate_to(lst, [](std::size_t i) { return static_cast<int>(i + 1); });

    std::vector<int> expected{1, 2, 3};
    size_t idx = 0;
    for (auto val : lst)
    {
        ASSERT_EQ(val, expected[idx++]);
    }
}

} // namespace piejam::algorithm
