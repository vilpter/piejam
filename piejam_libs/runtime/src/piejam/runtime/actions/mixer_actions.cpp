// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/actions/mixer_actions.h>

#include <piejam/functional/get.h>
#include <piejam/runtime/state.h>

#include <iterator>

namespace piejam::runtime::actions
{

void
add_mixer_channel::reduce(state& st) const
{
    auto added_mixer_channel_id =
            runtime::add_mixer_channel(st, channel_type, name);

    if (auto_assign_input && channel_type != mixer::channel_type::aux)
    {
        for (auto device_id : *st.external_audio_state.inputs)
        {
            auto it = std::ranges::find(
                    st.mixer_state.io_map.in(),
                    mixer::io_address_t{device_id},
                    get_by_index<1>);

            if (it == st.mixer_state.io_map.in().end() &&
                st.external_audio_state.devices[device_id].bus_type ==
                        to_bus_type(channel_type))
            {
                st.mixer_state.io_map.in().lock()[added_mixer_channel_id] =
                        device_id;
                break;
            }
        }
    }
}

void
delete_mixer_channel::reduce(state& st) const
{
    runtime::remove_mixer_channel(st, mixer_channel_id);
}

void
set_mixer_channel_color::reduce(state& st) const
{
    auto const& channel = st.mixer_state.channels[channel_id];
    st.material_colors.set(channel.color, color);
}

void
set_mixer_channel_route::reduce(state& st) const
{
    BOOST_ASSERT_MSG(
            !(st.mixer_state.channels[channel_id].type ==
                      mixer::channel_type::aux &&
              port == io_direction::input),
            "changing aux input is not allowed");
    st.mixer_state.io_map[port].lock()[channel_id] = route;
}

void
enable_mixer_channel_aux_route::reduce(state& st) const
{
    [this](mixer::channel& mixer_channel) {
        auto aux_sends = mixer_channel.aux_sends.lock();
        auto it = aux_sends->find(aux_id);
        BOOST_ASSERT(it != aux_sends->end());
        it->second.enabled = enabled;
    }(st.mixer_state.channels.lock()[channel_id]);
}

void
move_mixer_channel_left::reduce(state& st) const
{
    auto channel_ids = st.mixer_state.inputs.lock();

    auto it = std::ranges::find(*channel_ids, channel_id);
    BOOST_ASSERT(it != channel_ids->end());
    BOOST_ASSERT(it != channel_ids->begin());
    std::iter_swap(it, std::prev(it));
}

void
move_mixer_channel_right::reduce(state& st) const
{
    auto channel_ids = st.mixer_state.inputs.lock();

    auto it = std::ranges::find(*channel_ids, channel_id);
    BOOST_ASSERT(it != channel_ids->end());
    BOOST_ASSERT(std::next(it) != channel_ids->begin());
    std::iter_swap(it, std::next(it));
}

} // namespace piejam::runtime::actions
