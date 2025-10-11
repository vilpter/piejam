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

namespace piejam::gui
{

struct Root::Impl
{
    Impl(runtime::state_access const& state_access)
        : audioDeviceSettings{state_access}
        , audioInputSettings{state_access, io_direction::input}
        , audioOutputSettings{state_access, io_direction::output}
        , midiInputSettings{state_access}
        , mixer{state_access}
        , info{state_access}
        , log{state_access}
        , fxBrowser{state_access}
        , fxModule{state_access}
        , rootView{state_access}
    {
    }

    model::AudioDeviceSettings audioDeviceSettings;
    model::AudioInputOutputSettings audioInputSettings;
    model::AudioInputOutputSettings audioOutputSettings;
    model::MidiInputSettings midiInputSettings;
    model::Mixer mixer;
    model::Info info;
    model::Log log;
    model::FxBrowser fxBrowser;
    model::FxModuleView fxModule;
    model::RootView rootView;
};

Root::Root(runtime::state_access const& state_access)
    : m_impl{make_pimpl<Impl>(state_access)}
{
}

auto
Root::audioDeviceSettings() const noexcept -> model::AudioDeviceSettings*
{
    return &m_impl->audioDeviceSettings;
}

auto
Root::audioInputSettings() const noexcept -> model::AudioInputOutputSettings*
{
    return &m_impl->audioInputSettings;
}

auto
Root::audioOutputSettings() const noexcept -> model::AudioInputOutputSettings*
{
    return &m_impl->audioOutputSettings;
}

auto
Root::midiInputSettings() const noexcept -> model::MidiInputSettings*
{
    return &m_impl->midiInputSettings;
}

auto
Root::mixer() const noexcept -> model::Mixer*
{
    return &m_impl->mixer;
}

auto
Root::info() const noexcept -> model::Info*
{
    return &m_impl->info;
}

auto
Root::log() const noexcept -> model::Log*
{
    return &m_impl->log;
}

auto
Root::fxBrowser() const noexcept -> model::FxBrowser*
{
    return &m_impl->fxBrowser;
}

auto
Root::fxModule() const noexcept -> model::FxModuleView*
{
    return &m_impl->fxModule;
}

auto
Root::rootView() const noexcept -> model::RootView*
{
    return &m_impl->rootView;
}

} // namespace piejam::gui
