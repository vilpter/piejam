// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/state.h>

#include <piejam/runtime/enum_parameter.h>
#include <piejam/runtime/fader_mapping.h>
#include <piejam/runtime/internal_fx_module_factory.h>
#include <piejam/runtime/ladspa_fx/ladspa_fx_module.h>
#include <piejam/runtime/parameter/float_normalize.h>
#include <piejam/runtime/parameter_factory.h>

#include <piejam/audio/types.h>
#include <piejam/functional/operators.h>
#include <piejam/indexed_access.h>
#include <piejam/ladspa/port_descriptor.h>
#include <piejam/numeric/dB_convert.h>
#include <piejam/set_if.h>
#include <piejam/tuple_element_compare.h>

#include <boost/hof/match.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

#include <algorithm>
#include <format>
#include <ranges>

namespace piejam::runtime
{

namespace
{

constexpr auto
to_normalized_volume(float_parameter const&, float const value)
{
    return fader_mapping::to_normalized_dB_mapping<
            fader_mapping::volume,
            fader_mapping::min_gain_dB>(value);
}

constexpr auto
from_normalized_volume(float_parameter const&, float const norm_value) -> float
{
    return fader_mapping::from_normalized_dB_maping<
            fader_mapping::volume,
            fader_mapping::min_gain_dB>(norm_value);
}

constexpr auto
to_normalized_send(float_parameter const&, float const value)
{
    return fader_mapping::to_normalized_dB_mapping<
            fader_mapping::send,
            fader_mapping::min_gain_dB>(value);
}

constexpr auto
from_normalized_send(float_parameter const&, float const norm_value) -> float
{
    return fader_mapping::from_normalized_dB_maping<
            fader_mapping::send,
            fader_mapping::min_gain_dB>(norm_value);
}

auto
volume_to_string(float volume) -> std::string
{
    auto const volume_dB = numeric::to_dB(volume);
    return std::format("{:.1f} dB", volume_dB);
}

} // namespace

auto
make_initial_state() -> state
{
    state st;
    st.mixer_state.main =
            add_mixer_channel(st, mixer::channel_type::stereo, "Main");
    // main doesn't belong into inputs
    remove_erase(st.mixer_state.inputs, st.mixer_state.main);
    // reset io
    [](mixer::channel& main_channel) {
        main_channel.in = mixer::mix_input{};
        main_channel.out = default_t{};
    }(st.mixer_state.channels.lock()[st.mixer_state.main]);
    st.params[st.mixer_state.channels[st.mixer_state.main].record()].value.set(
            true);
    return st;
}

static auto
make_internal_fx_module(fx::modules_t& fx_modules, fx::module&& fx_mod)
{
    return fx_modules.emplace(std::move(fx_mod));
}

void
apply_parameter_values(
        std::span<parameter_value_assignment const> values,
        parameters_map const& parameters,
        parameters_store& params_store)
{
    for (auto&& [key, value] : values)
    {
        if (auto it = parameters.find(key); it != parameters.end())
        {
            auto const param_id = it->second;
            std::visit(
                    boost::hof::match(
                            [&params_store]<class P>(
                                    parameter::id_t<P> id,
                                    parameter::value_type_t<P> v) {
                                params_store[id].value.set(v);
                            },
                            [](auto&&, auto&&) { BOOST_ASSERT(false); }),
                    param_id,
                    value);
        }
    }
}

static void
update_midi_assignments(
        midi_assignments_map& midi_assigns,
        midi_assignments_map const& new_assignments)
{
    for (auto&& [id, ass] : new_assignments)
    {
        std::erase_if(midi_assigns, tuple::element<1>.equal_to(std::cref(ass)));

        midi_assigns.insert_or_assign(id, ass);
    }
}

void
apply_midi_assignments(
        std::span<parameter_midi_assignment const> midi_assigns,
        parameters_map const& parameters,
        midi_assignments_map& midi_assigns_store)
{
    midi_assignments_map new_assignments;
    for (auto&& [key, value] : midi_assigns)
    {
        if (auto it = parameters.find(key); it != parameters.end())
        {
            BOOST_ASSERT(
                    std::visit(
                            []<class ParamId>(ParamId) {
                                return is_midi_assignable_v<ParamId>;
                            },
                            it->second));
            new_assignments.emplace(it->second, value);
        }
    }

    update_midi_assignments(midi_assigns_store, new_assignments);
}

auto
insert_internal_fx_module(
        state& st,
        mixer::channel_id const mixer_channel_id,
        std::size_t const position,
        fx::internal_id fx_internal_id,
        std::vector<parameter_value_assignment> const& initial_values,
        std::vector<parameter_midi_assignment> const& midi_assigns)
        -> fx::module_id
{
    BOOST_ASSERT(mixer_channel_id.valid());

    mixer::channel const& mixer_channel =
            st.mixer_state.channels[mixer_channel_id];
    auto const bus_type = to_bus_type(mixer_channel.type);
    fx::chain_t fx_chain = st.mixer_state.fx_chains[mixer_channel_id];
    auto const insert_pos = std::min(position, fx_chain.size());

    fx::module_id fx_mod_id = make_internal_fx_module(
            st.fx_modules,
            internal_fx_module_factories::lookup(fx_internal_id)({
                    .bus_type = bus_type,
                    .sample_rate = st.sample_rate,
                    .params = st.params,
                    .streams = st.streams,
            }));

    fx_chain.emplace(std::next(fx_chain.begin(), insert_pos), fx_mod_id);

    auto const& fx_mod = st.fx_modules[fx_chain[insert_pos]];

    apply_parameter_values(initial_values, fx_mod.parameters, st.params);
    apply_midi_assignments(
            midi_assigns,
            fx_mod.parameters,
            *st.midi_assignments.lock());

    st.mixer_state.fx_chains.set(mixer_channel_id, box{std::move(fx_chain)});

    return fx_mod_id;
}

auto
insert_ladspa_fx_module(
        state& st,
        mixer::channel_id const mixer_channel_id,
        std::size_t const position,
        ladspa::instance_id const instance_id,
        ladspa::plugin_descriptor const& plugin_desc,
        std::span<ladspa::port_descriptor const> const control_inputs,
        std::vector<parameter_value_assignment> const& initial_values,
        std::vector<parameter_midi_assignment> const& midi_assigns)
        -> fx::module_id
{
    BOOST_ASSERT(mixer_channel_id != mixer::channel_id{});

    mixer::channel const& mixer_channel =
            st.mixer_state.channels[mixer_channel_id];
    auto const bus_type = to_bus_type(mixer_channel.type);
    fx::chain_t fx_chain = st.mixer_state.fx_chains[mixer_channel_id];
    auto const insert_pos = std::min(position, fx_chain.size());

    auto fx_mod_id = st.fx_modules.emplace(
            ladspa_fx::make_module(
                    instance_id,
                    plugin_desc.name,
                    bus_type,
                    control_inputs,
                    st.params));

    fx_chain.emplace(std::next(fx_chain.begin(), insert_pos), fx_mod_id);

    auto const& fx_mod = st.fx_modules[fx_chain[insert_pos]];
    apply_parameter_values(initial_values, fx_mod.parameters, st.params);
    apply_midi_assignments(
            midi_assigns,
            fx_mod.parameters,
            *st.midi_assignments.lock());

    st.mixer_state.fx_chains.set(mixer_channel_id, box{std::move(fx_chain)});

    st.fx_ladspa_instances.emplace(instance_id, plugin_desc);

    return fx_mod_id;
}

void
insert_missing_ladspa_fx_module(
        state& st,
        mixer::channel_id const channel_id,
        std::size_t const position,
        fx::unavailable_ladspa const& unavail,
        std::string_view const name)
{
    auto const& mixer_channel = st.mixer_state.channels[channel_id];

    fx::chain_t fx_chain = st.mixer_state.fx_chains[channel_id];

    auto id = st.fx_unavailable_ladspa_plugins.emplace(unavail);
    auto const insert_pos = std::min(position, fx_chain.size());
    fx_chain.emplace(
            std::next(fx_chain.begin(), insert_pos),
            st.fx_modules.emplace(
                    fx::module{
                            .fx_instance_id = id,
                            .name = box(std::string(name)),
                            .bus_type = to_bus_type(mixer_channel.type),
                            .parameters = {},
                            .streams = {},
                    }));

    st.mixer_state.fx_chains.set(channel_id, box{std::move(fx_chain)});
}

template <class P>
static auto
remove_midi_assignement_for_parameter(state& st, parameter::id_t<P> id)
{
    st.midi_assignments.lock()->erase(midi_assignment_id{id});
}

template <class P>
static auto
remove_parameter(state& st, parameter::id_t<P> id)
{
    st.params.remove(id);

    if constexpr (boost::mp11::mp_contains<
                          midi_assignment_id,
                          parameter::id_t<P>>::value)
    {
        remove_midi_assignement_for_parameter(st, id);
    }
}

static auto
remove_parameters(state& st, parameters_map const& params)
{
    for (auto&& [key, param_id] : params)
    {
        std::visit([&st](auto&& id) { remove_parameter(st, id); }, param_id);
    }
}

void
remove_fx_module(
        state& st,
        mixer::channel_id const fx_chain_id,
        fx::module_id const fx_mod_id)
{
    fx::module const& fx_mod = st.fx_modules[fx_mod_id];

    fx::chain_t fx_chain = st.mixer_state.fx_chains[fx_chain_id];
    BOOST_ASSERT(std::ranges::contains(fx_chain, fx_mod_id));
    boost::remove_erase(fx_chain, fx_mod_id);
    st.mixer_state.fx_chains.set(fx_chain_id, box{std::move(fx_chain)});

    remove_parameters(st, *fx_mod.parameters);

    if (auto id = std::get_if<ladspa::instance_id>(&fx_mod.fx_instance_id))
    {
        st.fx_ladspa_instances.erase(*id);
    }
    else if (
            auto id = std::get_if<fx::unavailable_ladspa_id>(
                    &fx_mod.fx_instance_id))
    {
        st.fx_unavailable_ladspa_plugins.erase(*id);
    }

    st.fx_modules.erase(fx_mod_id);
}

template <class ParameterFactory>
static auto
make_aux_send(
        mixer::aux_sends_t& aux_sends,
        mixer::channel_id const& aux_id,
        ParameterFactory& ui_params_factory)
{
    using namespace std::string_literals;
    aux_sends.emplace(
            aux_id,
            mixer::aux_send{
                    .enabled = false,
                    .fader_tap = ui_params_factory.make_parameter(
                            enum_parameter<mixer::aux_send_fader_tap>(
                                    "Fader Tap",
                                    &mixer::aux_send::to_fader_tap_string,
                                    false /* midi_assignable */,
                                    true /* routing */)),
                    .volume = ui_params_factory.make_parameter(
                            float_parameter{
                                    .name = box("Send"s),
                                    .default_value = 0.f,
                                    .min = 0.f,
                                    .max = 1.f,
                                    .value_to_string = &volume_to_string,
                                    .to_normalized = &to_normalized_send,
                                    .from_normalized =
                                            &from_normalized_send})});
}

template <class ParameterFactory>
static auto
make_aux_sends(
        mixer::channels_t const& channels,
        ParameterFactory& ui_params_factory)
{
    mixer::aux_sends_t result;
    for (auto const& [channel_id, channel] : channels)
    {
        if (channel.type == mixer::channel_type::aux)
        {
            make_aux_send(result, channel_id, ui_params_factory);
        }
    }

    return result;
}

static auto
remove_aux_send(
        state& st,
        mixer::aux_sends_t& aux_sends,
        mixer::channel_id const& aux_id)
{
    if (auto it = aux_sends.find(aux_id); it != aux_sends.end())
    {
        remove_parameter(st, it->second.volume);
        aux_sends.erase(it);
    }
}

auto
add_external_audio_device(
        state& st,
        std::string const& name,
        io_direction const io_dir,
        audio::bus_type const bus_type,
        channel_index_pair const& channels) -> external_audio::device_id
{
    auto boxed_name = box(name);

    auto name_id = string_id::generate();
    st.strings.insert(name_id, boxed_name);

    auto id = st.external_audio_state.devices.emplace(
            external_audio::device{
                    .name = name_id,
                    .bus_type = io_dir == io_direction::input
                                        ? bus_type
                                        : audio::bus_type::stereo,
                    .channels = channels,
            });

    auto& devices_ids = io_dir == io_direction::input
                                ? st.external_audio_state.inputs
                                : st.external_audio_state.outputs;

    emplace_back(devices_ids, id);

    return id;
}

auto
add_mixer_channel(state& st, mixer::channel_type type, std::string name)
        -> mixer::channel_id
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    auto name_id = string_id::generate();
    st.strings.insert(name_id, box{std::move(name)});

    auto color_id = material_color_id::generate();
    st.material_colors.insert(color_id, material_color::pink);

    parameter_factory params_factory{st.params};
    auto mixer_channels = st.mixer_state.channels.lock();
    auto channel_id = mixer_channels.emplace(
            mixer::channel{
                    .type = type,
                    .name = name_id,
                    .color = color_id,
                    .parameters = box{parameters_map_by<
                            mixer::channel::parameter_key>{
                            {mixer::channel::parameter_key::volume,
                             params_factory.make_parameter(
                                     float_parameter{
                                             .name = box("Volume"s),
                                             .default_value = 1.f,
                                             .min = 0.f,
                                             .max = numeric::from_dB(6.f),
                                             .value_to_string =
                                                     &volume_to_string,
                                             .to_normalized =
                                                     &to_normalized_volume,
                                             .from_normalized =
                                                     &from_normalized_volume})},
                            {mixer::channel::parameter_key::pan_balance,
                             params_factory.make_parameter(
                                     float_parameter{
                                             .name = box(
                                                     std::string(bool_enum_to(
                                                             to_bus_type(type),
                                                             "Pan"sv,
                                                             "Balance"sv))),
                                             .default_value = 0.f,
                                             .min = -1.f,
                                             .max = 1.f,
                                             .bipolar = true,
                                             .to_normalized =
                                                     &parameter::
                                                             to_normalized_linear,
                                             .from_normalized =
                                                     &parameter::
                                                             from_normalized_linear})},
                            {mixer::channel::parameter_key::record,
                             params_factory.make_parameter(
                                     bool_parameter{
                                             .name = box("Record"s),
                                             .default_value = false})},
                            {mixer::channel::parameter_key::mute,
                             params_factory.make_parameter(
                                     bool_parameter{
                                             .name = box("Mute"s),
                                             .default_value = false})},
                            {mixer::channel::parameter_key::solo,
                             params_factory.make_parameter(
                                     bool_parameter{
                                             .name = box("Solo"s),
                                             .default_value = false})},
                    }},
                    .out_stream = make_stream(st.streams, 2),
                    .in = type == mixer::channel_type::aux
                                  ? mixer::io_address_t{mixer::mix_input{}}
                                  : mixer::io_address_t{},
                    .out = st.mixer_state.main,
                    .aux_sends = type == mixer::channel_type::aux
                                         ? box{mixer::aux_sends_t{}}
                                         : box{make_aux_sends(
                                                   st.mixer_state.channels,
                                                   params_factory)},
            });
    emplace_back(st.mixer_state.inputs, channel_id);

    if (type == mixer::channel_type::aux)
    {
        // add as aux_send to each channel
        for (auto& [id, channel] : mixer_channels)
        {
            if (channel.type != mixer::channel_type::aux)
            {
                make_aux_send(
                        *channel.aux_sends.lock(),
                        channel_id,
                        params_factory);
            }

            st.mixer_state.aux_channels.emplace(
                    channel_id,
                    mixer::aux_channel{
                            .default_fader_tap = params_factory.make_parameter(
                                    enum_parameter<
                                            mixer::aux_channel_fader_tap>(
                                            "Fader Tap"s,
                                            &mixer::aux_channel::
                                                    to_fader_tap_string,
                                            false /* midi_assignable */,
                                            true /* routing */))});
        }
    }

    st.mixer_state.fx_chains.emplace(channel_id);

    return channel_id;
}

void
remove_mixer_channel(state& st, mixer::channel_id const mixer_channel_id)
{
    BOOST_ASSERT(mixer_channel_id != st.mixer_state.main);

    mixer::channel const& mixer_channel =
            st.mixer_state.channels[mixer_channel_id];

    st.strings.erase(mixer_channel.name);
    st.material_colors.erase(mixer_channel.color);

    remove_parameters(st, mixer_channel.parameters);

    auto mixer_channels = st.mixer_state.channels.lock();

    if (mixer_channel.type == mixer::channel_type::aux)
    {
        // remove own aux_sends
        for (auto const& aux_send : mixer_channel.aux_sends.get())
        {
            remove_parameter(st, aux_send.second.volume);
        }

        // remove itself as aux_send from other channels
        for (auto& [id, channel] : mixer_channels)
        {
            remove_aux_send(st, *channel.aux_sends.lock(), mixer_channel_id);
        }
    }

    for (auto fx_mod_id :
         std::views::reverse(*st.mixer_state.fx_chains[mixer_channel_id]))
    {
        remove_fx_module(st, mixer_channel_id, fx_mod_id);
    }

    if (st.focused_fx_chain_id == mixer_channel_id)
    {
        st.focused_fx_chain_id = {};
        st.focused_fx_mod_id = {};
    }

    if (st.fx_browser_fx_chain_id == mixer_channel_id)
    {
        st.fx_browser_fx_chain_id = mixer::channel_id{};
    }

    BOOST_ASSERT(
            std::ranges::contains(*st.mixer_state.inputs, mixer_channel_id));
    remove_erase(st.mixer_state.inputs, mixer_channel_id);

    st.streams.erase(mixer_channel.out_stream);

    auto const addr = mixer::io_address_t{mixer_channel_id};
    auto const equal_to_mixer_channel = equal_to(addr);
    for (auto& [_, channel] : mixer_channels)
    {
        set_if(channel.in, equal_to_mixer_channel, default_t{});
        set_if(channel.out, equal_to_mixer_channel, default_t{});
    }

    st.mixer_state.fx_chains.erase(mixer_channel_id);

    mixer_channels.erase(mixer_channel_id);
}

void
remove_external_audio_device(
        state& st,
        external_audio::device_id const device_id)
{
    auto const name_id = st.external_audio_state.devices[device_id].name;
    auto const name = st.strings[name_id];

    st.strings.erase(name_id);

    auto mixer_channels = st.mixer_state.channels.lock();
    {
        auto const equal_to_device = equal_to(mixer::io_address_t(device_id));

        for (auto& [_, mixer_channel] : mixer_channels)
        {
            set_if(mixer_channel.in, equal_to_device, default_t{});
            set_if(mixer_channel.out, equal_to_device, default_t{});
        }
    }

    st.external_audio_state.devices.erase(device_id);

    if (std::ranges::contains(*st.external_audio_state.inputs, device_id))
    {
        remove_erase(st.external_audio_state.inputs, device_id);
    }
    else
    {
        BOOST_ASSERT(
                std::ranges::contains(
                        *st.external_audio_state.outputs,
                        device_id));
        remove_erase(st.external_audio_state.outputs, device_id);
    }
}

void
update_midi_assignments(state& st, midi_assignments_map const& assignments)
{
    update_midi_assignments(*st.midi_assignments.lock(), assignments);
}

} // namespace piejam::runtime
