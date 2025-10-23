// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

namespace piejam::runtime
{

class state_access;

} // namespace piejam::runtime

namespace piejam::gui::model
{

class Root final : public CompositeSubscribableModel
{
    Q_OBJECT

    PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioDeviceSettings*,
        audioDeviceSettings)

    PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioInputOutputSettings*,
        audioInputSettings)

    PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioInputOutputSettings*,
        audioOutputSettings)

    PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::MidiInputSettings*,
        midiInputSettings)

    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Mixer*, mixer)
    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Info*, info)
    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Log*, log)
    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::FxBrowser*, fxBrowser)
    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::FxModuleView*, fxModule)

    PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::RootView*, rootView)

public:
    explicit Root(runtime::state_access const&);

private:
    void onSubscribe() override;
};

} // namespace piejam::gui::model
