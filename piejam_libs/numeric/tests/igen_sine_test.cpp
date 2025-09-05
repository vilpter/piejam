// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/numeric/igen/sine.h>

#include <gtest/gtest.h>

#include <vector>

namespace piejam::numeric::igen::test
{

TEST(igen_sine, generates_correct_amplitude)
{
    sine<double> gen(440.0, 48000.0, 0.5);
    auto value = gen(0);
    EXPECT_DOUBLE_EQ(value, 0.0); // phase 0, sin(0) = 0
}

TEST(igen_sine, respects_initial_phase)
{
    sine<double> gen(440.0, 48000.0, 1.0, std::numbers::pi_v<double> / 2.0);
    auto value = gen(0);
    EXPECT_DOUBLE_EQ(value, 1.0); // sin(π/2) = 1
}

TEST(igen_sine, generates_expected_values)
{
    sine<double> gen(1.0, 4.0); // 1 Hz, 4 samples per second
    std::vector<double> samples;
    for (std::size_t i = 0; i < 4; ++i)
    {
        samples.push_back(gen(i));
    }

    EXPECT_NEAR(samples[0], 0.0, std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(samples[1], 1.0, std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(samples[2], 0.0, std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(samples[3], -1.0, std::numeric_limits<double>::epsilon());
}

TEST(igen_sine, output_in_expected_range)
{
    sine<double> gen(440.0, 48000.0, 0.7);
    for (std::size_t i = 0; i < 1000; ++i)
    {
        auto value = gen(i);
        EXPECT_LE(value, 0.7);
        EXPECT_GE(value, -0.7);
    }
}

} // namespace piejam::numeric::igen::test
