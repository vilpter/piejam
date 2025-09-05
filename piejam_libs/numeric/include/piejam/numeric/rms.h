// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/numeric/pow_n.h>

#include <cmath>
#include <concepts>
#include <numeric>
#include <span>

namespace piejam::numeric
{

namespace detail
{

struct rms_fn
{
    template <std::floating_point T>
    constexpr auto operator()(std::span<T const> const in) const noexcept
    {
        if (in.empty())
        {
            return T{};
        }

        return std::sqrt(
                std::transform_reduce(
                        in.begin(),
                        in.end(),
                        T{},
                        std::plus<>{},
                        numeric::pow_n<2>) /
                in.size());
    }

    template <std::ranges::contiguous_range R>
        requires std::floating_point<std::ranges::range_value_t<R>>
    constexpr auto operator()(R const& in) const noexcept
    {
        using T = std::add_const_t<std::ranges::range_value_t<R>>;
        return operator()(std::span<T>{in});
    }
};

} // namespace detail

inline constexpr detail::rms_fn rms{};

} // namespace piejam::numeric
