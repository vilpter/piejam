// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <concepts>
#include <limits>
#include <numbers>

namespace piejam::numeric::constants
{

template <std::floating_point T>
inline constexpr T negative_inf = -std::numeric_limits<T>::infinity();

template <class T>
inline constexpr T two_pi = T{2} * std::numbers::pi_v<T>;

} // namespace piejam::numeric::constants
