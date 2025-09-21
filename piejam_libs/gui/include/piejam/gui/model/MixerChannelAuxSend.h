// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/MixerChannel.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/mixer_fwd.h>

class QAbstractListModel;

namespace piejam::gui::model
{

class MixerChannelAuxSend final : public MixerChannel
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, auxSends)

public:
    MixerChannelAuxSend(
            runtime::store_dispatch,
            runtime::subscriber&,
            runtime::mixer::channel_id);

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
