// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/MixerChannel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/mixer_fwd.h>

class QAbstractListModel;

namespace piejam::gui::model
{

class MixerChannelAuxSend final : public MixerChannel
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, sends)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::AuxChannel*, aux)

public:
    MixerChannelAuxSend(
        runtime::state_access const&,
        runtime::mixer::channel_id);

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
