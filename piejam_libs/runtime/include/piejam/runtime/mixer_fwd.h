// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/external_audio_fwd.h>

#include <piejam/audio/types.h>
#include <piejam/default.h>
#include <piejam/fwd.h>

#include <variant>
#include <vector>

namespace piejam::runtime::mixer
{

enum class channel_type
{
    mono,
    stereo,
    aux,
};

constexpr auto
to_bus_type(channel_type t) -> audio::bus_type
{
    return t == channel_type::mono ? audio::bus_type::mono
                                   : audio::bus_type::stereo;
}

struct channel;
using channel_id = entity_id<channel>;
using channels_t = entity_map<channel>;

struct mix_input
{
    constexpr auto operator==(mix_input const&) const noexcept
            -> bool = default;
};

using io_address_t = std::
        variant<default_t, mix_input, external_audio::device_id, channel_id>;

enum class io_socket : bool
{
    in,
    out,
};

enum class fader_tap : bool
{
    post,
    pre,
};

using channel_ids_t = std::vector<channel_id>;

struct state;

} // namespace piejam::runtime::mixer
