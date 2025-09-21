// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/Subscribable.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/mixer_fwd.h>

namespace piejam::gui::model
{

class AuxSend final : public Subscribable<SubscribableModel>
{
    Q_OBJECT

public:
    enum class FaderTap : bool
    {
        Post,
        Pre,
    };

    Q_ENUM(FaderTap)

private:
    M_PIEJAM_GUI_PROPERTY(QString, name, setName)
    M_PIEJAM_GUI_PROPERTY(bool, canToggle, setCanToggle)
    M_PIEJAM_GUI_PROPERTY(bool, enabled, setEnabled)
    M_PIEJAM_GUI_PROPERTY(FaderTap, faderTap, setFaderTap)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::FloatParameter*, volume)

public:
    AuxSend(runtime::store_dispatch,
            runtime::subscriber&,
            runtime::mixer::channel_id ch_id,
            runtime::mixer::channel_id aux_id);

    Q_INVOKABLE void toggleEnabled();
    Q_INVOKABLE void toggleFaderTap();

private:
    void onSubscribe() override;

    runtime::mixer::channel_id m_channel_id;
    runtime::mixer::channel_id m_aux_id;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
