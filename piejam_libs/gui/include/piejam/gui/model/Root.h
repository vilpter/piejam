// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>

#include <QObject>

namespace piejam::runtime
{

class state_access;

} // namespace piejam::runtime

namespace piejam::gui
{

class Root final : public QObject
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioDeviceSettings*,
        audioDeviceSettings)

    M_PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioInputOutputSettings*,
        audioInputSettings)

    M_PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::AudioInputOutputSettings*,
        audioOutputSettings)

    M_PIEJAM_GUI_CONSTANT_PROPERTY(
        piejam::gui::model::MidiInputSettings*,
        midiInputSettings)

    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Mixer*, mixer)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Info*, info)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::Log*, log)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::FxBrowser*, fxBrowser)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::FxModuleView*, fxModule)

    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::RootView*, rootView)

public:
    explicit Root(runtime::state_access const&);

private:
    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui
