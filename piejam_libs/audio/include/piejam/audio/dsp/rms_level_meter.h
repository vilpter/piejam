// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/audio/sample_rate.h>

#include <piejam/functional/operators.h>
#include <piejam/numeric/align.h>
#include <piejam/numeric/flush_to_zero_if.h>
#include <piejam/numeric/mipp_iterator.h>
#include <piejam/numeric/pow_n.h>
#include <piejam/numeric/simd/rolling_sum.h>

#include <mipp.h>

#include <algorithm>
#include <chrono>
#include <concepts>

namespace piejam::audio::dsp
{

template <std::floating_point T = float>
class rms_level_meter
{
    static constexpr std::chrono::milliseconds default_rms_measure_time{800};
    static constexpr float default_min_level = 0.001f; // -60 dB

public:
    using value_type = mipp::Reg<std::remove_const_t<T>>;

    explicit constexpr rms_level_meter(
            sample_rate sr,
            std::chrono::duration<T> rms_measure_time =
                    default_rms_measure_time,
            T min_level = default_min_level)
        : m_min_level{min_level}
        , m_rolling_sqr_sum(
                  numeric::align_down(
                          sr.samples_for_duration(rms_measure_time),
                          static_cast<std::size_t>(mipp::N<T>())))
    {
    }

    [[nodiscard]]
    constexpr auto window_size() const noexcept -> std::size_t
    {
        return m_rolling_sqr_sum.window_size();
    }

    constexpr void process(std::span<T const> samples) noexcept
    {
        m_rolling_sqr_sum.update(samples);
    }

    [[nodiscard]]
    constexpr auto level() const noexcept -> T
    {
        return numeric::flush_to_zero_if(
                std::sqrt(
                        std::max(m_rolling_sqr_sum.sum(), T{0}) /
                        m_sqr_history_size),
                less(m_min_level));
    }

    constexpr void reset() noexcept
    {
        m_rolling_sqr_sum.reset();
    }

private:
    T m_min_level;
    numeric::simd::rolling_sum<decltype(numeric::pow_n<2>), T>
            m_rolling_sqr_sum;

    T m_sqr_history_size{static_cast<T>(m_rolling_sqr_sum.window_size())};
};

} // namespace piejam::audio::dsp
