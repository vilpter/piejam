// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/ModelManager.h>

#include <piejam/gui/item/DbScale.h>
#include <piejam/gui/item/FixedLinearScaleGrid.h>
#include <piejam/gui/item/FixedLogScaleGrid.h>
#include <piejam/gui/item/Scope.h>
#include <piejam/gui/item/Spectrum.h>
#include <piejam/gui/item/Waveform.h>
#include <piejam/gui/model/AudioDeviceSettings.h>
#include <piejam/gui/model/AudioInputOutputSettings.h>
#include <piejam/gui/model/AudioRouting.h>
#include <piejam/gui/model/AudioRoutingSelection.h>
#include <piejam/gui/model/AudioStreamProvider.h>
#include <piejam/gui/model/AuxChannel.h>
#include <piejam/gui/model/AuxSend.h>
#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/DbScaleData.h>
#include <piejam/gui/model/EnumListModel.h>
#include <piejam/gui/model/EnumParameter.h>
#include <piejam/gui/model/ExternalAudioDeviceConfig.h>
#include <piejam/gui/model/FloatParameter.h>
#include <piejam/gui/model/FxBrowser.h>
#include <piejam/gui/model/FxChainModule.h>
#include <piejam/gui/model/FxModule.h>
#include <piejam/gui/model/FxModuleRegistry.h>
#include <piejam/gui/model/FxModuleView.h>
#include <piejam/gui/model/Info.h>
#include <piejam/gui/model/IntParameter.h>
#include <piejam/gui/model/Log.h>
#include <piejam/gui/model/MidiAssignable.h>
#include <piejam/gui/model/MidiDeviceConfig.h>
#include <piejam/gui/model/MidiInputSettings.h>
#include <piejam/gui/model/Mixer.h>
#include <piejam/gui/model/MixerChannel.h>
#include <piejam/gui/model/MixerChannelAuxSend.h>
#include <piejam/gui/model/MixerChannelEdit.h>
#include <piejam/gui/model/MixerChannelFx.h>
#include <piejam/gui/model/MixerChannelModels.h>
#include <piejam/gui/model/MixerChannelPerform.h>
#include <piejam/gui/model/MixerDbScales.h>
#include <piejam/gui/model/Parameter.h>
#include <piejam/gui/model/RootView.h>
#include <piejam/gui/model/ScopeSlot.h>
#include <piejam/gui/model/SpectrumSlot.h>
#include <piejam/gui/model/StereoLevel.h>
#include <piejam/gui/model/String.h>
#include <piejam/gui/model/StringList.h>
#include <piejam/gui/model/Types.h>
#include <piejam/gui/model/WaveformSlot.h>

#include <mutex>

namespace piejam::gui
{

static void
runRegistration()
{
    using namespace model;

    qRegisterMetaType<MaterialColor>("MaterialColor");

    qRegisterMetaType<piejam::gui::model::FxModuleType>();
    qRegisterMetaType<piejam::gui::model::FxModuleRegistryItem>();

    qRegisterMetaType<piejam::gui::model::AudioDeviceSettings*>();
    qRegisterMetaType<piejam::gui::model::AudioInputOutputSettings*>();
    qRegisterMetaType<piejam::gui::model::AudioRouting*>();
    qRegisterMetaType<piejam::gui::model::AudioRoutingSelection*>();
    qRegisterMetaType<piejam::gui::model::AudioStreamProvider*>();
    qRegisterMetaType<piejam::gui::model::AuxChannel*>();
    qRegisterMetaType<piejam::gui::model::AuxSend*>();
    qRegisterMetaType<piejam::gui::model::BoolParameter*>();
    qRegisterMetaType<piejam::gui::model::DbScaleData*>();
    qRegisterMetaType<piejam::gui::model::EnumListModel*>();
    qRegisterMetaType<piejam::gui::model::EnumParameter*>();
    qRegisterMetaType<piejam::gui::model::FloatParameter*>();
    qRegisterMetaType<piejam::gui::model::FxBrowser*>();
    qRegisterMetaType<piejam::gui::model::FxChainModule*>();
    qRegisterMetaType<piejam::gui::model::FxModule*>();
    qRegisterMetaType<piejam::gui::model::FxModuleView*>();
    qRegisterMetaType<piejam::gui::model::Info*>();
    qRegisterMetaType<piejam::gui::model::IntParameter*>();
    qRegisterMetaType<piejam::gui::model::Log*>();
    qRegisterMetaType<piejam::gui::model::MidiAssignable*>();
    qRegisterMetaType<piejam::gui::model::MidiInputSettings*>();
    qRegisterMetaType<piejam::gui::model::Mixer*>();
    qRegisterMetaType<piejam::gui::model::MixerChannel*>();
    qRegisterMetaType<piejam::gui::model::MixerChannelAuxSend*>();
    qRegisterMetaType<piejam::gui::model::MixerChannelEdit*>();
    qRegisterMetaType<piejam::gui::model::MixerChannelFx*>();
    qRegisterMetaType<piejam::gui::model::MixerChannelModels*>();
    qRegisterMetaType<piejam::gui::model::MixerChannelPerform*>();
    qRegisterMetaType<piejam::gui::model::Parameter*>();
    qRegisterMetaType<piejam::gui::model::RootView*>();
    qRegisterMetaType<piejam::gui::model::ScopeSlot*>();
    qRegisterMetaType<piejam::gui::model::SpectrumSlot*>();
    qRegisterMetaType<piejam::gui::model::StereoLevel*>();
    qRegisterMetaType<piejam::gui::model::String*>();
    qRegisterMetaType<piejam::gui::model::StringList*>();
    qRegisterMetaType<piejam::gui::model::WaveformSlot*>();

    qmlRegisterUncreatableType<piejam::gui::model::AudioRoutingSelection>(
            "PieJam.Models",
            1,
            0,
            "AudioRoutingSelection",
            "Not creatable");

    qmlRegisterUncreatableType<piejam::gui::model::FxModule>(
            "PieJam.Models",
            1,
            0,
            "FxModule",
            "Not createable");

    qmlRegisterUncreatableType<piejam::gui::model::AuxSend>(
            "PieJam.Models",
            1,
            0,
            "AuxSend",
            "Not creatable");

    qmlRegisterUncreatableType<piejam::gui::model::Parameter>(
            "PieJam.Models",
            1,
            0,
            "Parameter",
            "Not createable");

    qmlRegisterUncreatableType<piejam::gui::model::RootView>(
            "PieJam.Models",
            1,
            0,
            "RootView",
            "Not createable");

    qmlRegisterType<piejam::gui::item::Waveform>(
            "PieJam.Items",
            1,
            0,
            "Waveform");
    qmlRegisterType<piejam::gui::item::Spectrum>(
            "PieJam.Items",
            1,
            0,
            "Spectrum");
    qmlRegisterType<piejam::gui::item::Scope>("PieJam.Items", 1, 0, "Scope");
    qmlRegisterType<piejam::gui::item::DbScale>(
            "PieJam.Items",
            1,
            0,
            "DbScale");
    qmlRegisterType<piejam::gui::item::FixedLinearScaleGrid>(
            "PieJam.Items",
            1,
            0,
            "FixedLinearScaleGrid");
    qmlRegisterType<piejam::gui::item::FixedLogScaleGrid>(
            "PieJam.Items",
            1,
            0,
            "FixedLogScaleGrid");

    qmlRegisterUncreatableMetaObject(
            piejam::gui::model::staticMetaObject,
            "PieJam.Models",
            1,
            0,
            "Types",
            "Not creatable as it is an enum type");

    qmlRegisterSingletonInstance<piejam::gui::model::MixerDbScales>(
            "PieJam.Models",
            1,
            0,
            "MixerDbScales",
            &model::g_mixerDbScales);

    qmlRegisterSingletonInstance<piejam::gui::model::FxModuleRegistry>(
            "PieJam.Models",
            1,
            0,
            "FxModuleRegistry",
            &model::fxModuleRegistrySingleton());
}

struct ModelManager::Impl
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

ModelManager::ModelManager(runtime::state_access const& state_access)
    : m_impl{make_pimpl<Impl>(state_access)}
{
    static std::once_flag s_registered;
    std::call_once(s_registered, &runRegistration);
}

auto
ModelManager::audioDeviceSettings() const noexcept
        -> model::AudioDeviceSettings*
{
    return &m_impl->audioDeviceSettings;
}

auto
ModelManager::audioInputSettings() const noexcept
        -> model::AudioInputOutputSettings*
{
    return &m_impl->audioInputSettings;
}

auto
ModelManager::audioOutputSettings() const noexcept
        -> model::AudioInputOutputSettings*
{
    return &m_impl->audioOutputSettings;
}

auto
ModelManager::midiInputSettings() const noexcept -> model::MidiInputSettings*
{
    return &m_impl->midiInputSettings;
}

auto
ModelManager::mixer() const noexcept -> model::Mixer*
{
    return &m_impl->mixer;
}

auto
ModelManager::info() const noexcept -> model::Info*
{
    return &m_impl->info;
}

auto
ModelManager::log() const noexcept -> model::Log*
{
    return &m_impl->log;
}

auto
ModelManager::fxBrowser() const noexcept -> model::FxBrowser*
{
    return &m_impl->fxBrowser;
}

auto
ModelManager::fxModule() const noexcept -> model::FxModuleView*
{
    return &m_impl->fxModule;
}

auto
ModelManager::rootView() const noexcept -> model::RootView*
{
    return &m_impl->rootView;
}

} // namespace piejam::gui
