// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/fx/fwd.h>
#include <piejam/runtime/mixer_fwd.h>

namespace piejam::gui::model
{

class FxModuleView : public SubscribableModel
{
    Q_OBJECT

    PIEJAM_GUI_PROPERTY(MaterialColor, color, setColor)
    PIEJAM_GUI_PROPERTY(QString, chainName, setChainName)
    PIEJAM_GUI_PROPERTY(QString, name, setName)
    PIEJAM_GUI_PROPERTY(piejam::gui::model::BoolParameter*, active, setActive)

    Q_PROPERTY(
        piejam::gui::model::FxModule* content READ content NOTIFY contentChanged
            FINAL)

public:
    FxModuleView(runtime::state_access const&);

    auto content() noexcept -> FxModule*;

signals:
    void contentChanged();

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
