// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AuxSend.h>

#include <piejam/gui/model/FloatParameter.h>

#include <piejam/runtime/actions/mixer_actions.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct AuxSend::Impl
{
    Impl(runtime::state_access const& state_access,
         runtime::float_parameter_id volume_id)
        : volume{state_access, volume_id}
    {
    }

    FloatParameter volume;
};

AuxSend::AuxSend(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id ch_id,
        runtime::mixer::channel_id aux_id)
    : SubscribableModel{state_access}
    , m_channel_id{ch_id}
    , m_aux_id{aux_id}
    , m_impl{make_pimpl<Impl>(
              state_access,
              observe_once(
                      runtime::selectors::
                              make_mixer_channel_aux_volume_parameter_selector(
                                      ch_id,
                                      aux_id)))}
{
}

auto
AuxSend::volume() const noexcept -> volume_property_t
{
    return &m_impl->volume;
}

void
AuxSend::onSubscribe()
{
    setName(QString::fromStdString(observe_once(
            runtime::selectors::make_mixer_channel_name_string_selector(
                    m_aux_id))));

    observe(runtime::selectors::make_mixer_channel_aux_enabled_selector(
                    m_channel_id,
                    m_aux_id),
            [this](bool const enabled) { setEnabled(enabled); });

    observe(runtime::selectors::make_mixer_channel_aux_fader_tap_selector(
                    m_channel_id,
                    m_aux_id),
            [this](runtime::mixer::fader_tap tap) {
                setFaderTap(bool_enum_to<FaderTap>(tap));
            });

    observe(runtime::selectors::make_mixer_channel_can_toggle_aux_selector(
                    m_channel_id,
                    m_aux_id),
            [this](bool const x) { setCanToggle(x); });
}

void
AuxSend::toggleEnabled()
{
    runtime::actions::enable_mixer_channel_aux_route action;
    action.channel_id = m_channel_id;
    action.aux_id = m_aux_id;
    action.enabled = !m_enabled;
    dispatch(action);
}

void
AuxSend::toggleFaderTap()
{
    runtime::actions::toggle_mixer_channel_aux_fader_tap action;
    action.channel_id = m_channel_id;
    action.aux_id = m_aux_id;
    dispatch(action);
}

} // namespace piejam::gui::model
