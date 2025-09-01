// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

namespace piejam::algorithm
{

namespace detail
{

struct to_vector_fn
{
    template <std::ranges::input_range Range>
    auto operator()(Range&& rng) const
    {
        using value_t = std::ranges::range_value_t<Range>;
        std::vector<value_t> result;

        if constexpr (std::ranges::sized_range<Range>)
        {
            result.reserve(std::ranges::size(rng));
        }

               // move elements if necessary (supports move-only types)
        if constexpr (
                std::is_move_constructible_v<value_t> &&
                !std::is_copy_constructible_v<value_t>)
        {
            std::ranges::move(
                    std::forward<Range>(rng),
                    std::back_inserter(result));
        }
        else
        {
            std::ranges::copy(
                    std::forward<Range>(rng),
                    std::back_inserter(result));
        }

        return result;
    }
};

} // namespace detail

template <std::ranges::input_range Range>
auto
operator|(Range&& rng, detail::to_vector_fn const& f)
{
    return f(std::forward<Range>(rng));
}

inline constexpr detail::to_vector_fn to_vector{};

} // namespace piejam::algorithm
