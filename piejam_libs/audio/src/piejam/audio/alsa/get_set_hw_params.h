// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/audio/fwd.h>

#include <piejam/system/fwd.h>

#include <filesystem>

namespace piejam::audio::alsa
{

auto get_num_channels(std::filesystem::path const& device_path) -> unsigned;

auto
get_hw_params(sound_card_stream_descriptor const&, sample_rate, period_size)
        -> sound_card_stream_hw_params;

struct set_hw_params_result
{
    unsigned period_count{};
};

auto set_hw_params(
        system::device&,
        sound_card_config const&,
        sound_card_buffer_config const&) -> set_hw_params_result;

} // namespace piejam::audio::alsa
