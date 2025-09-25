// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/MixerChannelAuxSend.h>

#include <piejam/gui/ListModelEditScriptProcessor.h>
#include <piejam/gui/model/AuxSend.h>
#include <piejam/gui/model/ObjectListModel.h>

#include <piejam/algorithm/edit_script.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

struct MixerChannelAuxSend::Impl
{
    box<runtime::mixer::channel_ids_t> aux_ids;
    AuxSendsList auxSends;
};

MixerChannelAuxSend::MixerChannelAuxSend(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id)
    : MixerChannel{state_access, id}
    , m_impl{make_pimpl<Impl>()}
{
}

auto
MixerChannelAuxSend::auxSends() const noexcept -> auxSends_property_t
{
    return &m_impl->auxSends;
}

void
MixerChannelAuxSend::onSubscribe()
{
    MixerChannel::onSubscribe();

    observe(runtime::selectors::make_mixer_channel_aux_sends_selector(
                    channel_id()),
            [this](box<runtime::mixer::channel_ids_t> const& aux_ids) {
                algorithm::apply_edit_script(
                        algorithm::edit_script(*m_impl->aux_ids, *aux_ids),
                        ListModelEditScriptProcessor{
                                m_impl->auxSends,
                                [this](auto const& aux_id) {
                                    return std::make_unique<AuxSend>(
                                            state_access(),
                                            channel_id(),
                                            aux_id);
                                }});

                m_impl->aux_ids = aux_ids;
            });
}

} // namespace piejam::gui::model
