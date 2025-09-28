// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/audio_stream_id.h>
#include <piejam/runtime/external_audio_fwd.h>
#include <piejam/runtime/fx/fwd.h>
#include <piejam/runtime/material_color.h>
#include <piejam/runtime/mixer_fwd.h>
#include <piejam/runtime/parameters.h>
#include <piejam/runtime/string_id.h>

#include <piejam/audio/types.h>
#include <piejam/boxed_string.h>
#include <piejam/entity_map.h>

#include <boost/assert.hpp>

#include <vector>

namespace piejam::runtime::mixer
{

struct aux_send
{
    static auto to_fader_tap_string(int const v) -> std::string
    {
        using namespace std::string_literals;
        switch (v)
        {
            case std::to_underlying(aux_send_fader_tap::post):
                return "Post"s;

            case std::to_underlying(aux_send_fader_tap::pre):
                return "Pre"s;
        }

        return "Auto"s;
    }

    bool enabled{};
    enum_parameter_id fader_tap{};
    float_parameter_id volume{};
};

using aux_sends_t = std::map<channel_id, aux_send>;

struct channel
{
    channel_type type{};

    string_id name{};
    material_color_id color{};

    float_parameter_id volume{};
    float_parameter_id pan_balance{};
    bool_parameter_id record{};
    bool_parameter_id mute{};
    bool_parameter_id solo{};

    audio_stream_id out_stream{};

    io_address_t in{};
    io_address_t out{};

    box<aux_sends_t> aux_sends{};

    auto operator==(channel const&) const noexcept -> bool = default;
};

struct aux_channel
{
    static auto to_fader_tap_string(int const v) -> std::string
    {
        using namespace std::string_literals;
        return v == std::to_underlying(aux_channel_fader_tap::pre) ? "Pre"s
                                                                   : "Post"s;
    }

    int_parameter_id default_fader_tap;
};

struct state
{
    channels_t channels;

    box<channel_ids_t> inputs;
    channel_id main;

    using aux_channels_t = entity_data_map<channel_id, aux_channel>;
    aux_channels_t aux_channels;

    using fx_chains_t = entity_data_map<channel_id, box<fx::chain_t>>;
    fx_chains_t fx_chains;
};

auto is_mix_input_valid(channels_t const&, channel_id) -> bool;

auto can_toggle_aux(channels_t const&, channel_id, channel_id aux_id) -> bool;

auto valid_channels(io_socket, channels_t const&, channel_id)
        -> std::vector<channel_id>;

} // namespace piejam::runtime::mixer
