// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/fader_mapping.h>

#include <gtest/gtest.h>

namespace piejam::runtime::fader_mapping
{

struct volume_fader
{
    static constexpr auto mapping = volume;
    static constexpr float min_dB = min_gain_dB;
};

struct send_fader
{
    static constexpr auto mapping = send;
    static constexpr float min_dB = min_gain_dB;
};

template <class Fader>
struct fader_mapping_test : public ::testing::Test
{
    using fader = Fader;
};

using fader_mapping_types = ::testing::Types<volume_fader, send_fader>;

TYPED_TEST_SUITE(fader_mapping_test, fader_mapping_types);

// --- Helper function for round-trip testing ---
template <auto Mapping, float Min_dB>
static void
test_round_trip(float value)
{
    float norm = to_normalized_dB_mapping<Mapping, Min_dB>(value);
    float restored = from_normalized_dB_mapping<Mapping, Min_dB>(norm);
    // Allow small floating-point errors
    EXPECT_NEAR(restored, value, 1e-5f);
}

TYPED_TEST(fader_mapping_test, exact_mapping_points)
{
    constexpr auto arr = TypeParam::mapping;
    constexpr float min_dB = TypeParam::min_dB;

    for (auto const& point : arr)
    {
        float value = numeric::from_dB(point.dB);
        float norm = to_normalized_dB_mapping<arr, min_dB>(value);
        EXPECT_NEAR(norm, point.normalized, 1e-6f);

        float restored = from_normalized_dB_mapping<arr, min_dB>(norm);
        EXPECT_NEAR(restored, value, 1e-5f);
    }
}

TYPED_TEST(fader_mapping_test, mid_segment_values)
{
    constexpr auto arr = TypeParam::mapping;
    constexpr float min_dB = TypeParam::min_dB;

    for (size_t i = 0; i + 1 < arr.size(); ++i)
    {
        float dB_mid = (arr[i].dB + arr[i + 1].dB) / 2.f;
        float value = numeric::from_dB(dB_mid);

        float norm = to_normalized_dB_mapping<arr, min_dB>(value);
        EXPECT_GE(norm, arr[i].normalized);
        EXPECT_LE(norm, arr[i + 1].normalized);

        float restored = from_normalized_dB_mapping<arr, min_dB>(norm);
        EXPECT_NEAR(restored, value, 1e-5f);
    }
}

TYPED_TEST(fader_mapping_test, below_min_returns_zero)
{
    constexpr auto arr = TypeParam::mapping;
    constexpr float min_dB = TypeParam::min_dB;

    float value = numeric::from_dB(min_dB - 20.f);
    float norm = to_normalized_dB_mapping<arr, min_dB>(value);
    EXPECT_FLOAT_EQ(norm, 0.f);
}

TYPED_TEST(fader_mapping_test, above_max_returns_one)
{
    constexpr auto arr = TypeParam::mapping;
    constexpr float min_dB = TypeParam::min_dB;

    float last_dB = arr.back().dB;
    float value = numeric::from_dB(last_dB + 10.f);
    float norm = to_normalized_dB_mapping<arr, min_dB>(value);
    EXPECT_FLOAT_EQ(norm, 1.f);
}

TYPED_TEST(fader_mapping_test, round_trip_consistency)
{
    constexpr auto arr = TypeParam::mapping;
    constexpr float min_dB = TypeParam::min_dB;

    for (auto const& point : arr)
    {
        float value = numeric::from_dB(point.dB);
        test_round_trip<arr, min_dB>(value);
    }
}

} // namespace piejam::runtime::fader_mapping
