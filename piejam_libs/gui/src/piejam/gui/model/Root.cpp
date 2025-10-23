// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/Root.h>

#include <piejam/gui/model/AudioDeviceSettings.h>
#include <piejam/gui/model/AudioInputOutputSettings.h>
#include <piejam/gui/model/FxBrowser.h>
#include <piejam/gui/model/FxModuleView.h>
#include <piejam/gui/model/Info.h>
#include <piejam/gui/model/Log.h>
#include <piejam/gui/model/MidiInputSettings.h>
#include <piejam/gui/model/Mixer.h>
#include <piejam/gui/model/RootView.h>

namespace piejam::gui::model
{

Root::Root(runtime::state_access const& state_access)
    : CompositeSubscribableModel{state_access}
    , m_audioDeviceSettings{&addModel<AudioDeviceSettings>()}
    , m_audioInputSettings{&addModel<AudioInputOutputSettings>(
          io_direction::input)}
    , m_audioOutputSettings{&addModel<AudioInputOutputSettings>(
          io_direction::output)}
    , m_midiInputSettings{&addModel<MidiInputSettings>()}
    , m_mixer{&addModel<Mixer>()}
    , m_info{&addModel<Info>()}
    , m_log{&addModel<Log>()}
    , m_fxBrowser{&addModel<FxBrowser>()}
    , m_fxModule{&addModel<FxModuleView>()}
    , m_rootView{&addModel<RootView>()}
{
}

void
Root::onSubscribe()
{
}

} // namespace piejam::gui::model
