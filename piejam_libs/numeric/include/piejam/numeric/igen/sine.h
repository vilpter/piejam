// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <concepts>
#include <numbers>

namespace piejam::numeric::igen
{

template <std::floating_point T>
class sine
{
    static constexpr T two_pi = T{2} * std::numbers::pi_v<T>;

public:
    constexpr sine(
            T frequency,
            T sample_rate,
            T amplitude_ = T{1},
            T phase_ = T{0}) noexcept
        : m_amplitude(amplitude_)
        , m_phase(phase_)
        , m_phase_increment(two_pi * frequency / sample_rate)
    {
    }

    constexpr auto operator()(std::size_t i) const noexcept -> T
    {
        // Wrap phase to [0, 2π) to prevent drift
        auto current_phase = std::fmod(m_phase + m_phase_increment * i, two_pi);
        return m_amplitude * std::sin(current_phase);
    }

private:
    T m_amplitude;
    T m_phase;           // initial phase in radians
    T m_phase_increment; // 2*pi*frequency/sample_rate
};

} // namespace piejam::numeric::igen
