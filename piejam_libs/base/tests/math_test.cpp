// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/math.h>

#include <gtest/gtest.h>

namespace piejam::test
{

TEST(to_dB, default_min)
{
    EXPECT_FLOAT_EQ(math::negative_inf<float>, math::to_dB(0.f));
}

TEST(to_dB, custom_min)
{
    EXPECT_FLOAT_EQ(math::negative_inf<float>, math::to_dB(0.f, 0.1f));
    EXPECT_FLOAT_EQ(math::negative_inf<float>, math::to_dB(0.05f, 0.1f));
    EXPECT_FLOAT_EQ(math::negative_inf<float>, math::to_dB(0.1f, 0.1f));
}

TEST(from_dB, default_min)
{
    EXPECT_FLOAT_EQ(0.f, math::from_dB(math::negative_inf<float>));
}

TEST(from_dB, custom_min)
{
    EXPECT_FLOAT_EQ(0.f, math::from_dB(math::negative_inf<float>, -20.f));
    EXPECT_FLOAT_EQ(0.f, math::from_dB(-60.f, -20.f));
    EXPECT_FLOAT_EQ(0.f, math::from_dB(-20.f, -20.f));
}

TEST(almost_equal, equal_values_are_true)
{
    EXPECT_TRUE(math::almost_equal(1.0f, 1.0f));
    EXPECT_TRUE(math::almost_equal(0.0f, 0.0f));
    EXPECT_TRUE(math::almost_equal(-2.5, -2.5));
}

TEST(almost_equal, small_difference_is_true)
{
    EXPECT_TRUE(math::almost_equal(1.0f, 1.0f + 1e-7f));
    EXPECT_TRUE(math::almost_equal(1000.0, 1000.0 + 1e-10));
}

TEST(almost_equal, large_difference_is_false)
{
    EXPECT_FALSE(math::almost_equal(1.0f, 1.1f));
    EXPECT_FALSE(math::almost_equal(1000.0, 1001.0));
}

TEST(almost_equal, near_zero_absolute_tolerance)
{
    EXPECT_TRUE(math::almost_equal(0.0f, 1e-7f));
    EXPECT_FALSE(math::almost_equal(0.0f, 1e-3f));
}

TEST(almost_equal, works_with_float_and_double)
{
    float a = 1.0f, b = 1.0f + 1e-7f;
    double x = 1.0, y = 1.0 + 1e-12;
    EXPECT_TRUE(math::almost_equal(a, b));
    EXPECT_TRUE(math::almost_equal(x, y));
}

TEST(almost_equal, negative_and_positive_values)
{
    EXPECT_TRUE(math::almost_equal(-1.0f, -1.0f - 1e-7f));
    EXPECT_FALSE(math::almost_equal(-1.0, 1.0));
}

} // namespace piejam::test
