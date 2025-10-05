// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AuxChannel.h>

#include <piejam/gui/model/EnumParameter.h>

#include <piejam/runtime/actions/mixer_actions.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct AuxChannel::Impl
{
    Impl(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id aux_id)
        : defaultFaderTap{
              state_access,
              state_access.observe_once(
                  runtime::selectors::
                      make_aux_channel_default_fader_tap_parameter_selector(
                          aux_id))}
    {
    }

    EnumParameter defaultFaderTap;
};

AuxChannel::AuxChannel(
    runtime::state_access const& state_access,
    runtime::mixer::channel_id aux_id)
    : SubscribableModel{state_access}
    , m_aux_id{aux_id}
    , m_impl{make_pimpl<Impl>(state_access, aux_id)}
{
}

auto
AuxChannel::defaultFaderTap() const noexcept -> defaultFaderTap_property_t
{
    return &m_impl->defaultFaderTap;
}

void
AuxChannel::onSubscribe()
{
}

} // namespace piejam::gui::model
