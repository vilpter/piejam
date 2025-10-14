// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/mixer_fwd.h>

namespace piejam::gui::model
{

class AuxChannel final : public SubscribableModel
{
    Q_OBJECT

    PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::EnumParameter*,
        defaultFaderTap)

public:
    AuxChannel(runtime::state_access const&, runtime::mixer::channel_id aux_id);

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
