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
[[nodiscard]]
constexpr auto
almost_equal(T a, T b) noexcept -> bool
{
    constexpr T abs_eps =
            std::numeric_limits<T>::epsilon() * 100; // near-zero tolerance
    constexpr T rel_eps = static_cast<T>(1e-5);      // relative tolerance

    T const diff = (a > b) ? (a - b) : (b - a);
    if (diff <= abs_eps)
    {
        return true;
    }

    T const largest = (a > b) ? a : b;
    return diff <= largest * rel_eps;
}

template <std::floating_point T>
constexpr auto
exact_equal(T a, T b) noexcept -> bool
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    return a == b;
#pragma GCC diagnostic pop
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
