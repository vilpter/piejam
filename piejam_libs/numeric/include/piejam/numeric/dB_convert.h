// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/numeric/constants.h>

#include <cmath>
#include <concepts>
#include <limits>

namespace piejam::numeric
{

template <std::floating_point T>
[[nodiscard]]
constexpr auto
to_dB(T log, T min_log = T{1e-20}) noexcept -> T
{
    static_assert(std::numeric_limits<T>::is_iec559, "IEEE 754 required");
    return log <= min_log ? constants::negative_inf<T>
                          : std::log10(log) * T{20};
}

template <std::floating_point T>
[[nodiscard]]
constexpr auto
from_dB(T dB, T min_dB = constants::negative_inf<T>) noexcept -> T
{
    return dB <= min_dB ? T{} : std::pow(T{10}, dB / T{20});
}

} // namespace piejam::numeric
