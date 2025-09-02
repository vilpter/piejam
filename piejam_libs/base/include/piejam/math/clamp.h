// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <type_traits>

namespace piejam::math
{

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

} // namespace piejam::math
