// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>

namespace piejam::algorithm
{

template <std::ranges::sized_range Target, std::ranges::sized_range Source>
constexpr void
shift_push_back(Target& target, Source&& source)
{
    auto const target_size = std::ranges::size(target);
    auto const source_size = std::ranges::size(source);
    auto const count = std::min(target_size, source_size);

    auto target_begin = std::ranges::begin(target);
    auto target_end = std::ranges::end(target);

    // Shift existing elements to the left to make room at the end
    std::shift_left(target_begin, target_end, count);

    auto source_begin =
            std::ranges::next(std::ranges::begin(source), source_size - count);
    auto source_end = std::ranges::end(source);
    auto target_output = std::ranges::next(target_begin, target_size - count);

    using target_value_t = std::ranges::range_value_t<Target>;
    if constexpr (
            std::is_move_constructible_v<target_value_t> &&
            !std::is_copy_constructible_v<target_value_t>)
    {
        // move elements if the type is move-only
        std::ranges::move(source_begin, source_end, target_output);
    }
    else
    {
        // copy elements for copyable types
        std::ranges::copy(source_begin, source_end, target_output);
    }
}

} // namespace piejam::algorithm
