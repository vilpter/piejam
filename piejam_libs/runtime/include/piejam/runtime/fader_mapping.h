// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/functional/in_interval.h>
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
    float const value_dB = numeric::to_dB(value);
    if (value_dB <= Min_dB)
    {
        return 0.f;
    }

    auto first = std::ranges::begin(Mapping);
    if (value_dB < first->dB)
    {
        return numeric::linear_map(
                value_dB,
                Min_dB,
                first->dB,
                0.f,
                first->normalized);
    }

    auto lower = std::ranges::adjacent_find(
            Mapping,
            in_closed(value_dB),
            &normalized_dB_mapping::dB);
    BOOST_ASSERT(lower != std::ranges::end(Mapping));
    auto const upper = std::next(lower);

    return numeric::linear_map(
            value_dB,
            lower->dB,
            upper->dB,
            lower->normalized,
            upper->normalized);
}

template <auto Mapping, float Min_dB>
constexpr auto
from_normalized_dB_maping(float const norm_value)
{
    if (norm_value == 0.f)
    {
        return 0.f;
    }

    auto first = std::ranges::begin(Mapping);
    if (norm_value < first->normalized)
    {
        return numeric::from_dB(
                numeric::linear_map(
                        norm_value,
                        0.f,
                        first->normalized,
                        Min_dB,
                        first->dB));
    }

    auto lower = std::ranges::adjacent_find(
            Mapping,
            in_closed(norm_value),
            &normalized_dB_mapping::normalized);
    BOOST_ASSERT(lower != std::ranges::end(Mapping));
    auto const upper = std::next(lower);

    return numeric::from_dB(
            numeric::linear_map(
                    norm_value,
                    lower->normalized,
                    upper->normalized,
                    lower->dB,
                    upper->dB));
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
