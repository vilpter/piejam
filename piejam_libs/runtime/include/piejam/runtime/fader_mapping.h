// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/functional/in_interval.h>
#include <piejam/numeric/clamp.h>
#include <piejam/numeric/dB_convert.h>
#include <piejam/numeric/linear_map.h>

#include <boost/assert.hpp>

#include <algorithm>
#include <ranges>

#pragma once

namespace piejam::runtime::fader_mapping
{

struct normalized_dB_mapping
{
    float normalized{};
    float dB{};
};

template <auto Mapping, float Min_dB>
constexpr auto
to_normalized_dB_mapping(float const value)
{
    static_assert(Mapping.size() >= 2);
    static_assert(
            std::ranges::is_sorted(Mapping, {}, &normalized_dB_mapping::dB));

    float value_dB = numeric::to_dB(value);

    // Below minimum
    if (value_dB <= Min_dB)
    {
        return 0.f;
    }

    // Above last point
    if (value_dB >= Mapping.back().dB)
    {
        return 1.f;
    }

    // Binary search for upper bound
    auto upper_it = std::ranges::upper_bound(
            Mapping,
            value_dB,
            std::less<>{},
            &normalized_dB_mapping::dB);

    BOOST_ASSERT(upper_it != Mapping.begin() && upper_it != Mapping.end());
    auto lower_it = std::prev(upper_it);

    return numeric::clamp(
            numeric::linear_map(
                    value_dB,
                    lower_it->dB,
                    upper_it->dB,
                    lower_it->normalized,
                    upper_it->normalized),
            0.f,
            1.f);
}

template <auto Mapping, float Min_dB>
constexpr auto
from_normalized_dB_mapping(float const norm_value)
{
    static_assert(Mapping.size() >= 2);
    static_assert(
            std::ranges::is_sorted(Mapping, {}, &normalized_dB_mapping::dB));

    // Below zero
    if (norm_value <= 0.f)
    {
        return 0.f;
    }

    if (norm_value >= 1.f)
    {
        return numeric::from_dB(Mapping.back().dB);
    }

    // Binary search for upper bound in normalized space
    auto upper_it = std::ranges::upper_bound(
            Mapping,
            norm_value,
            std::less<>{},
            &normalized_dB_mapping::normalized);

    BOOST_ASSERT(upper_it != Mapping.begin() && upper_it != Mapping.end());
    auto lower_it = std::prev(upper_it);

    return numeric::from_dB(
            numeric::linear_map(
                    norm_value,
                    lower_it->normalized,
                    upper_it->normalized,
                    lower_it->dB,
                    upper_it->dB));
}

inline constexpr auto volume = std::array{
        normalized_dB_mapping{.normalized = 0.05f, .dB = -60.f},
        normalized_dB_mapping{.normalized = 0.3f, .dB = -30.f},
        normalized_dB_mapping{.normalized = 0.55f, .dB = -12.f},
        normalized_dB_mapping{.normalized = 1.f, .dB = 6.f},
};

inline constexpr auto send = std::array{
        normalized_dB_mapping{.normalized = 0.05f, .dB = -60.f},
        normalized_dB_mapping{.normalized = 0.3f, .dB = -30.f},
        normalized_dB_mapping{.normalized = 0.48f, .dB = -18.f},
        normalized_dB_mapping{.normalized = 1.f, .dB = 0.f},
};

inline constexpr auto min_gain_dB = -160.f;

} // namespace piejam::runtime::fader_mapping
