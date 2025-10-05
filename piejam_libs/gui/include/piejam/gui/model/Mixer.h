// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/runtime/mixer_fwd.h>

#include <piejam/pimpl.h>

class QAbstractListModel;

namespace piejam::gui::model
{

class Mixer final : public SubscribableModel
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, userChannels)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::MixerChannelModels*,
        mainChannel)

public:
    Mixer(runtime::state_access const&);

    Q_INVOKABLE void addMonoChannel(QString const& newChannelName);
    Q_INVOKABLE void addStereoChannel(QString const& newChannelName);
    Q_INVOKABLE void addAuxChannel(QString const& newChannelName);

private:
    void onSubscribe() override;

    void addChannel(
        QString const& name,
        runtime::mixer::channel_type,
        bool auto_assign_input);

    struct Impl;
    pimpl<Impl> const m_impl;
};

} // namespace piejam::gui::model
