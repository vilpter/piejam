// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/MixerChannelModels.h>

#include <piejam/gui/model/MixerChannelAuxSend.h>
#include <piejam/gui/model/MixerChannelEdit.h>
#include <piejam/gui/model/MixerChannelFx.h>
#include <piejam/gui/model/MixerChannelPerform.h>

namespace piejam::gui::model
{

struct MixerChannelModels::Impl
{
    Impl(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id)
        : m_perform{state_access, id}
        , m_edit{state_access, id}
        , m_fx{state_access, id}
        , m_auxSend{state_access, id}
    {
    }

    MixerChannelPerform m_perform;
    MixerChannelEdit m_edit;
    MixerChannelFx m_fx;
    MixerChannelAuxSend m_auxSend;
};

MixerChannelModels::MixerChannelModels(
    runtime::state_access const& state_access,
    runtime::mixer::channel_id const id)
    : SubscribableModel(state_access)
    , m_impl{make_pimpl<Impl>(state_access, id)}
{
}

void
MixerChannelModels::onSubscribe()
{
}

auto
MixerChannelModels::perform() const noexcept -> MixerChannelPerform*
{
    return &m_impl->m_perform;
}

auto
MixerChannelModels::edit() const noexcept -> MixerChannelEdit*
{
    return &m_impl->m_edit;
}

auto
MixerChannelModels::fx() const noexcept -> MixerChannelFx*
{
    return &m_impl->m_fx;
}

auto
MixerChannelModels::auxSend() const noexcept -> MixerChannelAuxSend*
{
    return &m_impl->m_auxSend;
}

} // namespace piejam::gui::model
