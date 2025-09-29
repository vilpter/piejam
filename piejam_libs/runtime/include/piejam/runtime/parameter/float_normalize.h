// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/float_descriptor.h>

#include <piejam/functional/in_interval.h>
#include <piejam/numeric/dB_convert.h>
#include <piejam/numeric/linear_map.h>

#include <algorithm>
#include <cmath>

namespace piejam::runtime::parameter
{

namespace detail
{

template <std::floating_point T>
[[nodiscard]]
constexpr auto
to_normalized(T const value, T const min, T const max) -> T
{
    return (value - min) / (max - min);
}

template <std::floating_point T>
[[nodiscard]]
constexpr auto
from_normalized(T const norm_value, T const min, T const max) -> T
{
    return norm_value * (max - min) + min;
}

using float_type = typename float_descriptor::value_type;

} // namespace detail

[[nodiscard]]
constexpr auto
to_normalized_linear(float_descriptor const& p, detail::float_type const value)
{
    return detail::to_normalized(value, p.min, p.max);
}

template <detail::float_type Min, detail::float_type Max>
[[nodiscard]]
constexpr auto
to_normalized_linear_static(
        float_descriptor const&,
        detail::float_type const value)
{
    return detail::to_normalized(value, Min, Max);
}

[[nodiscard]]
constexpr auto
from_normalized_linear(
        float_descriptor const& p,
        detail::float_type const norm_value)
{
    return detail::from_normalized(norm_value, p.min, p.max);
}

template <detail::float_type Min, detail::float_type Max>
[[nodiscard]]
constexpr auto
from_normalized_linear_static(
        float_descriptor const&,
        detail::float_type const norm_value)
{
    return detail::from_normalized(norm_value, Min, Max);
}

[[nodiscard]]
constexpr auto
to_normalized_log(float_descriptor const& p, detail::float_type const value)
{
    return detail::to_normalized(
            std::log(value),
            std::log(p.min),
            std::log(p.max));
}

[[nodiscard]]
constexpr auto
from_normalized_log(
        float_descriptor const& p,
        detail::float_type const norm_value)
{
    return std::exp(
            detail::from_normalized(
                    norm_value,
                    std::log(p.min),
                    std::log(p.max)));
}

template <detail::float_type Min_dB, detail::float_type Max_dB>
[[nodiscard]]
constexpr auto
to_normalized_dB(float_descriptor const&, detail::float_type const value)
{
    return detail::to_normalized(
            std::log10(value) * detail::float_type{20},
            Min_dB,
            Max_dB);
}

template <detail::float_type Min_dB, detail::float_type Max_dB>
[[nodiscard]]
constexpr auto
from_normalized_dB(float_descriptor const&, detail::float_type const norm_value)
{
    return std::pow(
            detail::float_type{10},
            detail::from_normalized(norm_value, Min_dB, Max_dB) /
                    detail::float_type{20});
}

} // namespace piejam::runtime::parameter
