// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/audio/fwd.h>

#include <piejam/thread/cpu_clock.h>

namespace piejam::audio
{

class cpu_load_meter
{
public:
    cpu_load_meter(std::size_t num_frames, sample_rate);

    auto stop() -> float;

private:
    using clock_t = thread::cpu_clock;
    std::chrono::duration<float, clock_t::period> const m_max_processing_time;
    clock_t::time_point const m_processing_start;
};

} // namespace piejam::audio
