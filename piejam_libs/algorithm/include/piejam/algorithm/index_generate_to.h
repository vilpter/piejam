// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <ranges>

namespace piejam::algorithm
{

template <typename F>
concept index_generator = requires(F f, std::size_t i) {
    requires !std::same_as<decltype(f(i)), void>;
};

template <typename F, typename R>
concept index_generator_for_range = requires(F f, std::size_t i) {
    requires std::convertible_to<decltype(f(i)), std::ranges::range_value_t<R>>;
};

template <index_generator F>
constexpr auto
index_generator_view(F&& f)
{
    return std::views::iota(std::size_t{0}) |
           std::views::transform(std::forward<F>(f));
}

template <std::ranges::range R, index_generator_for_range<R> F>
    requires std::output_iterator<
            std::ranges::iterator_t<R>,
            std::invoke_result_t<F&, std::size_t>>
constexpr void
index_generate_to(R&& r, F&& f)
{
    std::ranges::copy(
            index_generator_view(std::forward<F>(f)) |
                    std::views::take(std::ranges::size(r)),
            std::ranges::begin(r));
}

} // namespace piejam::algorithm
