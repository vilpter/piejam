// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/algorithm/transform_accumulate.h>

#include <gtest/gtest.h>

#include <numeric>
#include <vector>

namespace piejam::algorithm::test
{

TEST(transform_accumulate_test, empty_input)
{
    std::vector<double> input;
    std::vector<double> output(input.size());

    auto sum = transform_accumulate(
            input.begin(),
            input.end(),
            output.begin(),
            0.,
            [](double x) { return x * x; },
            std::plus<double>{});

    EXPECT_EQ(sum, 0.);
    EXPECT_TRUE(output.empty());
}

TEST(transform_accumulate_test, constant_input)
{
    std::vector<double> input(10, double{2});
    std::vector<double> output(input.size());

    auto sum = transform_accumulate(
            input.begin(),
            input.end(),
            output.begin(),
            0.,
            [](double x) { return x * x; },
            std::plus<double>{});

    // sum of squares: 10 * 2*2 = 40
    EXPECT_EQ(sum, double{40});
    for (auto val : output)
    {
        EXPECT_EQ(val, double{4});
    }
}

TEST(transform_accumulate_test, sequential_input)
{
    std::vector<double> input(5);
    std::iota(input.begin(), input.end(), double{1}); // 1,2,3,4,5
    std::vector<double> output(input.size());

    auto sum = transform_accumulate(
            input.begin(),
            input.end(),
            output.begin(),
            0.,
            [](double x) { return x + 1.; }, // transform: +1
            std::plus<double>{});

    // transformed output: 2,3,4,5,6 -> sum = 20
    EXPECT_EQ(sum, double{20});
    std::vector<double> expected_output{2, 3, 4, 5, 6};
    EXPECT_EQ(output, expected_output);
}

} // namespace piejam::algorithm::test
