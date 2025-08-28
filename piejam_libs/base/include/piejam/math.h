// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/assert.hpp>
#include <boost/hof/lift.hpp>
#include <boost/hof/returns.hpp>

#include <cmath>
#include <concepts>
#include <functional>
#include <limits>

namespace piejam::math
{

template <std::floating_point T>
constexpr T negative_inf = -std::numeric_limits<T>::infinity();

template <std::floating_point T>
[[nodiscard]]
constexpr auto
to_dB(T log, T min_log = T{1e-20}) noexcept -> T
{
    static_assert(std::numeric_limits<T>::is_iec559, "IEEE 754 required");
    return log <= min_log ? negative_inf<T> : std::log10(log) * T{20};
}

template <std::floating_point T>
[[nodiscard]]
constexpr auto
from_dB(T dB, T min_dB = negative_inf<T>) noexcept -> T
{
    return dB <= min_dB ? T{} : std::pow(T{10}, dB / T{20});
}

template <class T>
    requires(std::is_arithmetic_v<T>)
[[nodiscard]]
constexpr auto
clamp(T v, T lo, T hi) noexcept -> T
{
    if (v < lo)
    {
        v = lo;
    }

    if (v > hi)
    {
        v = hi;
    }

    return v;
}

template <class T, std::predicate<T> P>
[[nodiscard]]
constexpr auto flush_to_zero_if(T value, P&& p) BOOST_HOF_RETURNS(
        std::invoke(std::forward<P>(p), value) ? T{0} : value);

template <std::floating_point T>
[[nodiscard]]
constexpr auto
linear_map(T v, T src_lo, T src_hi, T dst_lo, T dst_hi) noexcept -> T
{
    BOOST_ASSERT(src_lo != src_hi);
    return ((v - src_lo) / (src_hi - src_lo)) * (dst_hi - dst_lo) + dst_lo;
}

template <std::integral T>
[[nodiscard]]
constexpr auto
pos_mod(T x, T y) noexcept
{
    BOOST_ASSERT(y != 0);

    T const n = x % y;

    if constexpr (std::signed_integral<T>)
    {
        return n < 0 ? n + y : n;
    }
    else
    {
        return n;
    }
}

inline constexpr auto abs = BOOST_HOF_LIFT(std::abs);

template <std::unsigned_integral T, std::unsigned_integral N>
[[nodiscard]]
constexpr auto
round_down_to_multiple(T x, N n) noexcept -> T
{
    BOOST_ASSERT(n != 0);
    return (x / n) * n;
}

} // namespace piejam::math
