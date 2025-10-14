// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AudioRouting.h>

#include <piejam/gui/ListModelEditScriptProcessor.h>
#include <piejam/gui/model/AudioRoutingSelection.h>
#include <piejam/gui/model/ObjectListModel.h>
#include <piejam/gui/model/String.h>

#include <piejam/algorithm/edit_script.h>
#include <piejam/runtime/actions/fwd.h>
#include <piejam/runtime/actions/mixer_actions.h>
#include <piejam/runtime/selectors.h>
#include <piejam/runtime/ui/thunk_action.h>

namespace piejam::gui::model
{

struct AudioRouting::Impl
{
    Impl(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id,
        io_direction const io_dir)
        : mixer_channel_id{id}
        , io_dir{io_dir}
        , channel_type{state_access.observe_once(
              runtime::selectors::make_mixer_channel_type_selector(id))}
        , selected{state_access, id, io_dir}
    {
    }

    runtime::mixer::channel_id mixer_channel_id;
    io_direction io_dir;
    runtime::mixer::channel_type channel_type;
    boxed_vector<runtime::selectors::mixer_device_route> devices{};
    boxed_vector<runtime::selectors::mixer_channel_route> channels{};

    AudioRoutingSelection selected;
    Strings devicesList{};
    Strings channelsList{};
};

AudioRouting::AudioRouting(
    runtime::state_access const& state_access,
    runtime::mixer::channel_id const id,
    io_direction const io_dir)
    : SubscribableModel(state_access)
    , m_impl{make_pimpl<Impl>(state_access, id, io_dir)}
{
    attachChildModel(m_impl->selected);
}

auto
AudioRouting::mixIsAvailable() const noexcept -> mixIsValid_property_t
{
    return m_impl->channel_type != runtime::mixer::channel_type::mono &&
           m_impl->io_dir == io_direction::input;
}

auto
AudioRouting::selected() const noexcept -> AudioRoutingSelection*
{
    return &m_impl->selected;
}

auto
AudioRouting::devices() const noexcept -> QAbstractListModel*
{
    return &m_impl->devicesList;
}

auto
AudioRouting::channels() const noexcept -> QAbstractListModel*
{
    return &m_impl->channelsList;
}

void
AudioRouting::onSubscribe()
{
    if (mixIsAvailable())
    {
        observe(
            runtime::selectors::make_mixer_channel_mix_input_is_valid_selector(
                m_impl->mixer_channel_id),
            [this](bool const x) { setMixIsValid(x); });
    }

    observe(
        runtime::selectors::make_mixer_device_routes_selector(
            m_impl->channel_type,
            m_impl->io_dir),
        [this](
            boxed_vector<runtime::selectors::mixer_device_route> const&
                devices) {
            algorithm::apply_edit_script(
                algorithm::edit_script(*m_impl->devices, *devices),
                ListModelEditScriptProcessor{
                    m_impl->devicesList,
                    [this](auto const& route) {
                        return std::make_unique<String>(
                            state_access(),
                            route.name);
                    }});

            m_impl->devices = devices;
        });

    observe(
        runtime::selectors::make_mixer_channel_routes_selector(
            m_impl->mixer_channel_id,
            m_impl->io_dir),
        [this](
            boxed_vector<runtime::selectors::mixer_channel_route> const&
                channels) {
            algorithm::apply_edit_script(
                algorithm::edit_script(*m_impl->channels, *channels),
                ListModelEditScriptProcessor{
                    m_impl->channelsList,
                    [this](auto const& route) {
                        return std::make_unique<String>(
                            state_access(),
                            route.name);
                    }});

            m_impl->channels = channels;
        });
}

static void
dispatch_set_mixer_channel_route_action(
    runtime::state_access state_access,
    runtime::mixer::channel_id channel_id,
    io_direction port,
    runtime::mixer::io_address_t addr)
{
    runtime::actions::set_mixer_channel_route action;
    action.channel_id = channel_id;
    action.port = port;
    action.route = std::move(addr);
    state_access.dispatch(action);
}

void
AudioRouting::changeToNone()
{
    dispatch_set_mixer_channel_route_action(
        state_access(),
        m_impl->mixer_channel_id,
        m_impl->io_dir,
        {});
}

void
AudioRouting::changeToMix()
{
    BOOST_ASSERT(mixIsAvailable() && mixIsValid());
    dispatch_set_mixer_channel_route_action(
        state_access(),
        m_impl->mixer_channel_id,
        m_impl->io_dir,
        runtime::mixer::mix_input{});
}

void
AudioRouting::changeToDevice(unsigned index)
{
    BOOST_ASSERT(index < m_impl->devices->size());

    dispatch_set_mixer_channel_route_action(
        state_access(),
        m_impl->mixer_channel_id,
        m_impl->io_dir,
        (*m_impl->devices)[index].device_id);
}

void
AudioRouting::changeToChannel(unsigned index)
{
    BOOST_ASSERT(index < m_impl->channels->size());

    dispatch_set_mixer_channel_route_action(
        state_access(),
        m_impl->mixer_channel_id,
        m_impl->io_dir,
        (*m_impl->channels)[index].channel_id);
}

} // namespace piejam::gui::model
