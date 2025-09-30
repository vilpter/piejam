// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/persistence/access.h>

#include <piejam/runtime/fx/unavailable_ladspa.h>
#include <piejam/runtime/persistence/app_config.h>
#include <piejam/runtime/persistence/session.h>
#include <piejam/runtime/state.h>

#include <piejam/algorithm/index_of.h>
#include <piejam/algorithm/transform_to_vector.h>

#include <spdlog/spdlog.h>

#include <boost/assert.hpp>
#include <boost/hof/match.hpp>
#include <boost/hof/unpack.hpp>

#include <filesystem>
#include <fstream>

namespace piejam::runtime::persistence
{

void
save_app_config(
        std::filesystem::path const& file,
        std::vector<std::string> const& enabled_midi_input_devices,
        state const& state)
{
    try
    {
        std::ofstream out(file);
        if (!out.is_open())
        {
            throw std::runtime_error("could not open config file");
        }

        persistence::app_config conf;

        conf.sound_card =
                state.selected_sound_card.index != npos
                        ? state.sound_cards
                                  .get()[state.selected_sound_card.index]
                                  .name
                        : std::string();
        conf.sample_rate = state.sample_rate;
        conf.period_size = state.period_size;

        conf.enabled_midi_input_devices = enabled_midi_input_devices;

        conf.rec_session = state.rec_session + 1;

        persistence::save_app_config(out, conf);
    }
    catch (std::exception const& err)
    {
        auto const* const message = err.what();
        spdlog::error("could not save config file: {}", message);
    }
}

namespace
{

auto
export_external_audio_device_configs(
        external_audio::devices_t const& devices,
        external_audio::device_ids_t const& device_ids,
        strings_t const& strings)
{
    return algorithm::transform_to_vector(
            device_ids,
            [&](external_audio::device_id const& device_id)
                    -> persistence::session::external_audio_device_config {
                external_audio::device const& device = devices.at(device_id);
                return {.name = strings[device.name],
                        .bus_type = device.bus_type,
                        .channels = device.channels};
            });
}

auto
export_parameter_values(
        parameters_map const& parameters,
        parameters_store const& params_store)
        -> std::vector<parameter_value_assignment>
{
    std::vector<parameter_value_assignment> result;

    for (auto&& [key, param_id] : parameters)
    {
        std::visit(
                [&]<class ParamId>(ParamId param_id) {
                    if constexpr (is_persistable_parameter_v<ParamId>)
                    {
                        result.emplace_back(
                                key,
                                params_store[param_id].value.get());
                    }
                },
                param_id);
    }

    return result;
}

auto
export_midi_assignments(
        parameters_map const& parameters,
        midi_assignments_map const& midi_assigns)
        -> std::vector<parameter_midi_assignment>
{
    std::vector<parameter_midi_assignment> result;

    for (auto&& [key, param_id] : parameters)
    {
        std::visit(
                [&](auto&& param_id) {
                    if (auto it = midi_assigns.find(param_id);
                        it != midi_assigns.end())
                    {
                        result.emplace_back(key, it->second);
                    }
                },
                param_id);
    }

    return result;
}

auto
export_fx_plugin(
        state const& st,
        fx::module const& fx_mod,
        fx::internal_id const fx_internal_id) -> session::fx_plugin
{
    BOOST_ASSERT(
            std::get<fx::internal_id>(fx_mod.fx_instance_id) == fx_internal_id);

    session::internal_fx fx;
    fx.type = fx_internal_id;
    fx.preset = export_parameter_values(fx_mod.parameters, st.params);
    fx.midi = export_midi_assignments(fx_mod.parameters, st.midi_assignments);
    return fx;
}

auto
export_fx_plugin(
        state const& st,
        fx::module const& fx_mod,
        ladspa::instance_id const id) -> session::fx_plugin
{
    BOOST_ASSERT(std::get<ladspa::instance_id>(fx_mod.fx_instance_id) == id);

    session::ladspa_plugin plug;
    auto const& pd = st.fx_ladspa_instances.at(id);
    plug.id = pd.id;
    plug.name = pd.name;
    plug.preset = export_parameter_values(fx_mod.parameters, st.params);
    plug.midi = export_midi_assignments(fx_mod.parameters, st.midi_assignments);
    return plug;
}

auto
export_fx_plugin(
        state const& st,
        fx::module const& fx_mod,
        fx::unavailable_ladspa_id const id) -> session::fx_plugin
{
    BOOST_ASSERT(
            std::get<fx::unavailable_ladspa_id>(fx_mod.fx_instance_id) == id);

    auto const& unavail = st.fx_unavailable_ladspa_plugins.at(id);
    session::ladspa_plugin plug;
    plug.id = unavail.plugin_id;
    plug.name = fx_mod.name;
    plug.preset = unavail.parameter_values;
    plug.midi = unavail.midi_assignments;
    return plug;
}

auto
export_fx_chain(state const& st, fx::chain_t const& fx_chain)
        -> persistence::session::fx_chain_t
{
    persistence::session::fx_chain_t result;

    for (auto const& fx_mod_id : fx_chain)
    {
        fx::module const& fx_mod = st.fx_modules.at(fx_mod_id);
        result.emplace_back(
                std::visit(
                        [&st, &fx_mod](auto const& id) {
                            return export_fx_plugin(st, fx_mod, id);
                        },
                        fx_mod.fx_instance_id));
    }

    return result;
}

auto
channel_index(mixer::state const& st, mixer::channel_id const channel_id)
{
    return channel_id == st.main
                   ? 0
                   : algorithm::index_of(*st.inputs, channel_id) + 1;
}

auto
device_index(
        external_audio::state const& st,
        external_audio::device_id const device_id)
{
    auto const in_index = algorithm::index_of(*st.io_ids.in(), device_id);
    return in_index != npos ? in_index
                            : algorithm::index_of(*st.io_ids.out(), device_id);
}

auto
export_mixer_io(state const& st, mixer::io_address_t const& addr)
{
    return std::visit(
            boost::hof::match(
                    [](default_t) {
                        return session::mixer_io{
                                .type = session::mixer_io_type::none,
                                .index = npos,
                        };
                    },
                    [](mixer::mix_input) {
                        return session::mixer_io{
                                .type = session::mixer_io_type::mix,
                                .index = npos,
                        };
                    },
                    [&st](mixer::channel_id channel_id) {
                        return session::mixer_io{
                                .type = session::mixer_io_type::channel,
                                .index = channel_index(
                                        st.mixer_state,
                                        channel_id),
                        };
                    },
                    [&st](external_audio::device_id device_id) {
                        return session::mixer_io{
                                .type = session::mixer_io_type::device,
                                .index = device_index(
                                        st.external_audio_state,
                                        device_id),
                        };
                    }),
            addr);
}

auto
export_mixer_aux_sends(state const& st, mixer::channel_id const channel_id)
{
    std::vector<session::mixer_aux_send> result;

    if (auto const* const channel_aux_sends =
                st.mixer_state.aux_sends.find(channel_id))
    {
        for (auto const& [aux, aux_send] : *channel_aux_sends)
        {
            result.emplace_back(
                    session::mixer_aux_send{
                            .channel_index = channel_index(st.mixer_state, aux),
                            .enabled = st.params[aux_send.active].value.get(),
                            .fader_tap =
                                    st.params[aux_send.fader_tap].value.get(),
                            .volume = st.params[aux_send.volume].value.get(),
                    });
        }
    }

    return result;
}

auto
export_mixer_channel(
        state const& st,
        mixer::channel_id const channel_id,
        mixer::channel const& channel)
{
    session::mixer_channel result;
    result.name = st.strings[channel.name];
    result.color = st.material_colors[channel.color];
    result.channel_type = channel.type;
    result.fx_chain =
            export_fx_chain(st, *st.mixer_state.fx_chains[channel_id]);
    result.midi =
            export_midi_assignments(channel.parameters, st.midi_assignments);
    result.parameter = export_parameter_values(channel.parameters, st.params);
    result.in = export_mixer_io(st, st.mixer_state.io_map.in().at(channel_id));
    result.out =
            export_mixer_io(st, st.mixer_state.io_map.out().at(channel_id));
    result.aux_sends = export_mixer_aux_sends(st, channel_id);
    return result;
}

auto
export_mixer_channels(state const& st, mixer::channel_ids_t const& channel_ids)
{
    return algorithm::transform_to_vector(
            channel_ids,
            [&st](mixer::channel_id const channel_id) {
                return export_mixer_channel(
                        st,
                        channel_id,
                        st.mixer_state.channels.at(channel_id));
            });
}

auto
export_aux_channel(
        state const& st,
        mixer::channel_id aux_id,
        mixer::aux_channel const& aux)
{
    session::aux_channel result;
    result.channel_index = channel_index(st.mixer_state, aux_id);
    result.fader_tap = st.params[aux.default_fader_tap].value.get();
    return result;
}

auto
export_aux_channels(state const& st)
{
    return algorithm::transform_to_vector(
            st.mixer_state.aux_channels,
            boost::hof::unpack([&st](mixer::channel_id aux_id,
                                     mixer::aux_channel const& aux) {
                return export_aux_channel(st, aux_id, aux);
            }));
}

} // namespace

void
save_session(std::filesystem::path const& file, state const& st)
{
    try
    {
        std::ofstream out(file);
        if (!out.is_open())
        {
            throw std::runtime_error("could not open session file");
        }

        session ses;

        ses.external_audio_input_devices = export_external_audio_device_configs(
                st.external_audio_state.devices,
                st.external_audio_state.io_ids.in().get(),
                st.strings);

        ses.external_audio_output_devices =
                export_external_audio_device_configs(
                        st.external_audio_state.devices,
                        st.external_audio_state.io_ids.out().get(),
                        st.strings);

        ses.mixer_channels = export_mixer_channels(st, *st.mixer_state.inputs);
        ses.main_mixer_channel = export_mixer_channel(
                st,
                st.mixer_state.main,
                st.mixer_state.channels.at(st.mixer_state.main));

        ses.aux_channels = export_aux_channels(st);

        save_session(out, ses);
    }
    catch (std::exception const& err)
    {
        auto const* const message = err.what();
        spdlog::error("save_session: {}", message);
    }
}

} // namespace piejam::runtime::persistence
