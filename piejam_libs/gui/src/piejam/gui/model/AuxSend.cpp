// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AuxSend.h>

#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/EnumParameter.h>
#include <piejam/gui/model/FloatParameter.h>

#include <piejam/runtime/actions/mixer_actions.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct AuxSend::Impl
{
    Impl(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id ch_id,
        runtime::mixer::channel_id aux_id)
        : channel_id{ch_id}
        , aux_id{aux_id}
        , active{state_access, state_access.observe_once(runtime::selectors::make_aux_send_active_selector(ch_id, aux_id))}
        , faderTap{state_access, state_access.observe_once(runtime::selectors::make_aux_send_fader_tap_selector(ch_id, aux_id))}
        , volume{
              state_access,
              state_access.observe_once(
                  runtime::selectors::make_aux_send_volume_parameter_selector(
                      ch_id,
                      aux_id))}
    {
    }

    runtime::mixer::channel_id channel_id;
    runtime::mixer::channel_id aux_id;

    BoolParameter active;
    EnumParameter faderTap;
    FloatParameter volume;
};

AuxSend::AuxSend(
    runtime::state_access const& state_access,
    runtime::mixer::channel_id ch_id,
    runtime::mixer::channel_id aux_id)
    : SubscribableModel{state_access}
    , m_impl{make_pimpl<Impl>(state_access, ch_id, aux_id)}
{
}

auto
AuxSend::active() const noexcept -> active_property_t
{
    return &m_impl->active;
}

auto
AuxSend::faderTap() const noexcept -> faderTap_property_t
{
    return &m_impl->faderTap;
}

auto
AuxSend::volume() const noexcept -> volume_property_t
{
    return &m_impl->volume;
}

void
AuxSend::onSubscribe()
{
    setName(
        QString::fromStdString(observe_once(
            runtime::selectors::make_mixer_channel_name_string_selector(
                m_impl->aux_id))));

    observe(
        runtime::selectors::make_can_toggle_aux_send_selector(
            m_impl->channel_id,
            m_impl->aux_id),
        [this](bool const x) { setCanToggle(x); });
}

} // namespace piejam::gui::model
