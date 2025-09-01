// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/algorithm/to_vector.h>

#include <ranges>

namespace piejam::algorithm
{

template <std::ranges::input_range Range, class F>
[[nodiscard]]
auto
transform_to_vector(Range&& rng, F&& f)
{
    return to_vector(
            std::views::transform(
                    std::forward<Range>(rng),
                    std::forward<F>(f)));
}

} // namespace piejam::algorithm
