// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/MixerChannelAuxSend.h>

#include <piejam/gui/ListModelEditScriptProcessor.h>
#include <piejam/gui/model/AuxChannel.h>
#include <piejam/gui/model/AuxSend.h>
#include <piejam/gui/model/ObjectListModel.h>

#include <piejam/algorithm/edit_script.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct MixerChannelAuxSend::Impl
{
    box<runtime::mixer::channel_ids_t> send_ids;

    std::unique_ptr<AuxChannel> auxChannel;
    std::unique_ptr<AuxSendsList> auxSends;
};

MixerChannelAuxSend::MixerChannelAuxSend(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id)
    : MixerChannel{state_access, id}
    , m_impl{make_pimpl<Impl>()}
{
    if (channelType() == ChannelType::Aux)
    {
        m_impl->auxChannel = std::make_unique<AuxChannel>(state_access, id);
    }
    else
    {
        m_impl->auxSends = std::make_unique<AuxSendsList>();
    }
}

auto
MixerChannelAuxSend::sends() const noexcept -> sends_property_t
{
    return m_impl->auxSends.get();
}

auto
MixerChannelAuxSend::aux() const noexcept -> aux_property_t
{
    return m_impl->auxChannel.get();
}

void
MixerChannelAuxSend::onSubscribe()
{
    MixerChannel::onSubscribe();

    if (channelType() != ChannelType::Aux)
    {
        observe(runtime::selectors::make_mixer_channel_aux_sends_selector(
                        channel_id()),
                [this](box<runtime::mixer::channel_ids_t> const& send_ids) {
                    algorithm::apply_edit_script(
                            algorithm::edit_script(
                                    *m_impl->send_ids,
                                    *send_ids),
                            ListModelEditScriptProcessor{
                                    *m_impl->auxSends,
                                    [this](auto const& send_id) {
                                        return std::make_unique<AuxSend>(
                                                state_access(),
                                                channel_id(),
                                                send_id);
                                    }});

                    m_impl->send_ids = send_ids;
                });
    }
}

} // namespace piejam::gui::model
