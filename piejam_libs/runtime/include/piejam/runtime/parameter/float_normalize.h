// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/float_descriptor.h>

#include <piejam/numeric/dB_convert.h>

#include <cmath>

namespace piejam::runtime::parameter
{

namespace detail
{

template <std::floating_point N, std::floating_point T>
[[nodiscard]]
constexpr auto
to_normalized(T const value, T const min, T const max) -> N
{
    return static_cast<N>((value - min) / (max - min));
}

template <std::floating_point N, std::floating_point T>
[[nodiscard]]
constexpr auto
from_normalized(N const norm_value, T const min, T const max) -> T
{
    return static_cast<T>(norm_value * (max - min) + min);
}

} // namespace detail

[[nodiscard]]
constexpr auto
to_normalized_linear(
        float_descriptor const& p,
        typename float_descriptor::value_type const value)
{
    return detail::to_normalized<typename float_descriptor::normalized_type>(
            value,
            p.min,
            p.max);
}

template <
        typename float_descriptor::value_type Min,
        typename float_descriptor::value_type Max>
[[nodiscard]]
constexpr auto
to_normalized_linear_static(
        float_descriptor const&,
        typename float_descriptor::value_type const value)
{
    return detail::to_normalized<typename float_descriptor::normalized_type>(
            value,
            Min,
            Max);
}

[[nodiscard]]
constexpr auto
from_normalized_linear(
        float_descriptor const& p,
        typename float_descriptor::normalized_type const norm_value)
{
    return detail::from_normalized(norm_value, p.min, p.max);
}

template <
        typename float_descriptor::value_type Min,
        typename float_descriptor::value_type Max>
[[nodiscard]]
constexpr auto
from_normalized_linear_static(
        float_descriptor const&,
        typename float_descriptor::normalized_type const norm_value)
{
    return detail::from_normalized(norm_value, Min, Max);
}

[[nodiscard]]
constexpr auto
to_normalized_log(
        float_descriptor const& p,
        typename float_descriptor::value_type const value)
{
    return detail::to_normalized<typename float_descriptor::normalized_type>(
            std::log(value),
            std::log(p.min),
            std::log(p.max));
}

[[nodiscard]]
constexpr auto
from_normalized_log(
        float_descriptor const& p,
        typename float_descriptor::normalized_type const norm_value)
{
    return std::exp(
            detail::from_normalized(
                    norm_value,
                    std::log(p.min),
                    std::log(p.max)));
}

template <
        typename float_descriptor::value_type Min_dB,
        typename float_descriptor::value_type Max_dB>
[[nodiscard]]
constexpr auto
to_normalized_dB(
        float_descriptor const&,
        typename float_descriptor::value_type const value)
{
    return detail::to_normalized<typename float_descriptor::normalized_type>(
            numeric::to_dB(value),
            Min_dB,
            Max_dB);
}

template <
        typename float_descriptor::value_type Min_dB,
        typename float_descriptor::value_type Max_dB>
[[nodiscard]]
constexpr auto
from_normalized_dB(
        float_descriptor const&,
        typename float_descriptor::normalized_type const norm_value)
{
    return numeric::from_dB(
            detail::from_normalized(norm_value, Min_dB, Max_dB));
}

} // namespace piejam::runtime::parameter
