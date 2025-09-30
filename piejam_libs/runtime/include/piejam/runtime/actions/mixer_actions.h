// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/actions/audio_engine_action.h>
#include <piejam/runtime/actions/ladspa_fx_action.h>
#include <piejam/runtime/fwd.h>
#include <piejam/runtime/material_color.h>
#include <piejam/runtime/mixer_fwd.h>
#include <piejam/runtime/ui/action.h>
#include <piejam/runtime/ui/cloneable_action.h>

#include <piejam/audio/types.h>
#include <piejam/boxed_string.h>
#include <piejam/entity_id.h>

namespace piejam::runtime::actions
{

struct add_mixer_channel final
    : ui::cloneable_action<add_mixer_channel, reducible_action>
    , visitable_audio_engine_action<add_mixer_channel>
{
    std::string name;
    mixer::channel_type channel_type;
    bool auto_assign_input{};

    void reduce(state&) const override;
};

struct delete_mixer_channel final
    : ui::cloneable_action<delete_mixer_channel, reducible_action>
    , visitable_audio_engine_action<delete_mixer_channel>
    , visitable_ladspa_fx_action<delete_mixer_channel>
{
    mixer::channel_id mixer_channel_id;

    void reduce(state&) const override;
};

struct set_mixer_channel_color final
    : ui::cloneable_action<set_mixer_channel_color, reducible_action>
{
    mixer::channel_id channel_id;
    material_color color{material_color::pink};

    void reduce(state&) const override;
};

struct set_mixer_channel_route final
    : ui::cloneable_action<set_mixer_channel_route, reducible_action>
    , visitable_audio_engine_action<set_mixer_channel_route>
{
    mixer::channel_id channel_id;
    io_direction port;
    mixer::io_address_t route;

    void reduce(state&) const override;
};

struct move_mixer_channel_left final
    : ui::cloneable_action<move_mixer_channel_left, reducible_action>
{
    mixer::channel_id channel_id;

    void reduce(state&) const override;
};

struct move_mixer_channel_right final
    : ui::cloneable_action<move_mixer_channel_right, reducible_action>
{
    mixer::channel_id channel_id;

    void reduce(state&) const override;
};

} // namespace piejam::runtime::actions
