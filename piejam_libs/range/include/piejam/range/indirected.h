// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <ranges>

namespace piejam::range
{

inline constexpr auto indirected = std::views::transform(
    [](auto&& x) -> decltype(auto) { return *std::forward<decltype(x)>(x); });

} // namespace piejam::range
