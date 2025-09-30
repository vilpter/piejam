// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/audio_stream_id.h>
#include <piejam/runtime/external_audio_fwd.h>
#include <piejam/runtime/fx/fwd.h>
#include <piejam/runtime/material_color.h>
#include <piejam/runtime/mixer_fwd.h>
#include <piejam/runtime/parameters_map.h>
#include <piejam/runtime/string_id.h>

#include <piejam/audio/types.h>
#include <piejam/boxed_map.h>
#include <piejam/boxed_string.h>
#include <piejam/entity_data_map.h>
#include <piejam/entity_map.h>
#include <piejam/io_direction.h>
#include <piejam/io_pair.h>

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

    bool_parameter_id active{};
    enum_parameter_id fader_tap{};
    float_parameter_id volume{};

    constexpr auto operator==(aux_send const&) const noexcept -> bool = default;
};

struct channel
{
    channel_type type{};

    string_id name{};
    material_color_id color{};

    enum class parameter_key : parameter::key
    {
        volume,
        pan_balance,
        record,
        mute,
        solo,
    };

    box<parameters_map_by<parameter_key>> parameters{};

    audio_stream_id out_stream{};

    auto operator==(channel const&) const noexcept -> bool = default;

    auto volume() const -> float_parameter_id
    {
        return parameters->get<float_parameter_id>(parameter_key::volume);
    }

    auto pan_balance() const -> float_parameter_id
    {
        return parameters->get<float_parameter_id>(parameter_key::pan_balance);
    }

    auto record() const -> bool_parameter_id
    {
        return parameters->get<bool_parameter_id>(parameter_key::record);
    }

    auto mute() const -> bool_parameter_id
    {
        return parameters->get<bool_parameter_id>(parameter_key::mute);
    }

    auto solo() const -> bool_parameter_id
    {
        return parameters->get<bool_parameter_id>(parameter_key::solo);
    }
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

    constexpr auto operator==(aux_channel const&) const noexcept
            -> bool = default;
};

struct state
{
    channels_t channels;

    box<channel_ids_t> inputs;
    channel_id main;

    mixer::io_map io_map;

    aux_channels_t aux_channels;

    aux_sends_t aux_sends;

    using fx_chains_t = entity_data_map<channel_id, box<fx::chain_t>>;
    fx_chains_t fx_chains;
};

auto is_mix_input_valid(
        channel_id,
        channels_t const&,
        io_map const&,
        aux_sends_t const&,
        parameters_store const&) -> bool;

auto can_toggle_aux(
        channel_id,
        channel_id aux_id,
        channels_t const&,
        io_map const&,
        aux_sends_t const&,
        parameters_store const&) -> bool;

auto valid_channels(
        channel_id,
        io_direction,
        channels_t const&,
        io_map const&,
        aux_sends_t const&,
        parameters_store const&) -> std::vector<channel_id>;

} // namespace piejam::runtime::mixer
