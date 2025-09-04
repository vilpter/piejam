// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/audio/sample_rate.h>

#include <piejam/algorithm/transform_accumulate.h>
#include <piejam/functional/operators.h>
#include <piejam/math.h>
#include <piejam/numeric/mipp_iterator.h>
#include <piejam/numeric/pow_n.h>

#include <mipp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
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

    explicit rms_level_meter(
            sample_rate sr,
            std::chrono::duration<T> rms_measure_time =
                    default_rms_measure_time,
            T min_level = default_min_level)
        : m_min_level{min_level}
        , m_sqr_history(
                  math::round_down_to_multiple(
                          sr.samples_for_duration(rms_measure_time),
                          static_cast<std::size_t>(mipp::N<T>())),
                  0.f)
    {
    }

    [[nodiscard]]
    auto history_size() const noexcept -> std::size_t
    {
        return m_sqr_history.size();
    }

    void process(std::span<T const> samples)
    {
        auto samples_data = samples.data();
        BOOST_ASSERT(mipp::isAligned(samples_data));

        std::size_t samples_size = samples.size();
        BOOST_ASSERT(samples_size % mipp::N<T>() == 0);

        std::size_t history_size = m_sqr_history.size();
        BOOST_ASSERT(m_position % mipp::N<T>() == 0);

        if (samples_size < history_size)
        {
            auto [lo, hi] = ring_buffer_split(samples_size);

            auto lo_mipp = numeric::mipp_range(lo);
            auto hi_mipp = numeric::mipp_range(hi);
            auto mid_it = numeric::mipp_iterator{samples_data + lo.size()};

            process_part<mipp::Reg<T>>(
                    numeric::mipp_iterator{samples_data},
                    mid_it,
                    lo_mipp.begin());

            if (hi.size() > 0)
            {
                process_part<mipp::Reg<T>>(
                        mid_it,
                        numeric::mipp_iterator{samples_data + samples_size},
                        hi_mipp.begin());
            }

            advance_position(samples_size);
        }
        else
        {
            m_sqr_sum = mipp::sum(
                    algorithm::transform_accumulate(
                            numeric::mipp_iterator{
                                    samples_data + samples_size - history_size},
                            numeric::mipp_iterator{samples_data + samples_size},
                            numeric::mipp_iterator{m_sqr_history.data()},
                            mipp::Reg<T>(T{}),
                            numeric::pow_n<2>,
                            std::plus<>{}));
            m_position = 0;
        }
    }

    [[nodiscard]]
    auto level() const noexcept -> T
    {
        return math::flush_to_zero_if(
                std::sqrt(std::max(m_sqr_sum, T{0}) / m_sqr_history_size),
                less(m_min_level));
    }

    void reset()
    {
        std::ranges::fill(m_sqr_history, T{0});
        m_sqr_sum = 0.f;
    }

private:
    template <class V, class SamplesIterator, class SqrIterator>
    void process_part(
            SamplesIterator samples_first,
            SamplesIterator samples_last,
            SqrIterator sqr_first)
    {
        V sub(T{0});
        V add(T{0});

        std::transform(
                samples_first,
                samples_last,
                sqr_first,
                sqr_first,
                [&](auto sample, auto old_sqr) {
                    sub += old_sqr;
                    auto new_sqr = sample * sample;
                    add += new_sqr;
                    return new_sqr;
                });

        adapt_sqr_sum(mipp::sum(sub), mipp::sum(add));
    }

    void adapt_sqr_sum(T sub, T add)
    {
        m_sqr_sum = m_sqr_sum - sub + add;
    }

    void advance_position(std::size_t num_samples)
    {
        m_position += num_samples;

        std::size_t history_size = m_sqr_history.size();
        if (m_position >= history_size)
        {
            m_position -= history_size;
        }
    }

    auto ring_buffer_split(std::size_t num_samples)
            -> std::tuple<std::span<T>, std::span<T>>
    {
        auto const history_data = m_sqr_history.data();
        auto const history_size = m_sqr_history.size();
        auto const first = history_data + m_position;

        if (m_position + num_samples < history_size)
        {
            auto last = first + num_samples;
            return std::tuple{std::span{first, last}, std::span{last, last}};
        }
        else
        {
            auto size = history_size - m_position;
            return std::tuple{
                    std::span{first, size},
                    std::span{history_data, num_samples - size}};
        }
    }

    T m_min_level;
    mipp::vector<T> m_sqr_history;

    std::size_t m_position{};
    T m_sqr_history_size{static_cast<T>(m_sqr_history.size())};
    T m_sqr_sum{};
};

} // namespace piejam::audio::dsp
