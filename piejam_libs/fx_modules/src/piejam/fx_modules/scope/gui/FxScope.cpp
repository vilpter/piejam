// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/fx_modules/scope/gui/FxScope.h>

#include <piejam/fx_modules/scope/scope_internal_id.h>
#include <piejam/fx_modules/scope/scope_module.h>

#include <piejam/audio/types.h>
#include <piejam/gui/model/AudioStreamProvider.h>
#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/EnumParameter.h>
#include <piejam/gui/model/FloatParameter.h>
#include <piejam/gui/model/ScopeGenerator.h>
#include <piejam/gui/model/StreamProcessor.h>
#include <piejam/gui/model/StreamSamplesCache.h>
#include <piejam/gui/model/WaveformGenerator.h>
#include <piejam/renew.h>
#include <piejam/runtime/parameters_map.h>
#include <piejam/runtime/selectors.h>

namespace piejam::fx_modules::scope::gui
{

using namespace piejam::gui::model;

namespace
{

struct ScopeStreamProcessor : StreamProcessor<ScopeStreamProcessor>
{
    template <class Samples>
    void process(Samples&& samples)
    {
        if (processAsWaveform)
        {
            waveform.update(
                    waveformGenerator.process(std::forward<Samples>(samples)));
        }
        else
        {
            scopeCache.process(std::forward<Samples>(samples));
        }
    }

    bool processAsWaveform{true};

    WaveformGenerator waveformGenerator;
    StreamSamplesCache scopeCache;

    WaveformSlot waveform;
    ScopeSlot scope;
};

} // namespace

struct FxScope::Impl
{
    static constexpr audio::sample_rate default_sample_rate{48000u};

    Impl(BusType busType)
        : busType{busType}
    {
    }

    BusType busType;
    audio::sample_rate sample_rate{default_sample_rate};

    std::unique_ptr<EnumParameter> mode;
    std::unique_ptr<EnumParameter> triggerSlope;
    std::unique_ptr<FloatParameter> triggerLevel;
    std::unique_ptr<FloatParameter> holdTime;
    std::unique_ptr<IntParameter> waveformResolution;
    std::unique_ptr<IntParameter> scopeResolution;
    std::unique_ptr<BoolParameter> freeze;
    std::unique_ptr<AudioStreamProvider> stream;

    std::pair<ScopeStreamProcessor, ScopeStreamProcessor> streamProcessor;
    ScopeGenerator scopeGenerator;

    auto modeEnum() const noexcept -> Mode
    {
        return mode->as<Mode>();
    }

    auto triggerSlopeEnum() const noexcept -> TriggerSlope
    {
        return triggerSlope->as<TriggerSlope>();
    }

    auto holdTimeInFrames() const noexcept -> std::size_t
    {
        return sample_rate.samples_for_duration(
                std::chrono::milliseconds{static_cast<int>(holdTime->value())});
    }

    void updateSampleRate(audio::sample_rate sr)
    {
        if (sr.valid())
        {
            sample_rate = sr;
        }
    }

    void updateMode()
    {
        bool processAsWaveform = modeEnum() == Mode::Free;
        streamProcessor.first.processAsWaveform = processAsWaveform;
        streamProcessor.second.processAsWaveform = processAsWaveform;
    }

    void updateWaveformGenerator()
    {
        int samplesPerPixel = std::pow(2, waveformResolution->value() * 3);
        renew(streamProcessor.first.waveformGenerator, samplesPerPixel);
        renew(streamProcessor.second.waveformGenerator, samplesPerPixel);
    }

    void updateWaveform(std::size_t viewSize)
    {
        streamProcessor.first.waveform.resize(viewSize);
        streamProcessor.second.waveform.resize(viewSize);
    }

    void updateScopeSamplesCache(std::size_t viewSize)
    {

        std::size_t doubleViewSize = viewSize * 2;
        int stride = scopeResolution->value() * 2 + 1;
        renew(streamProcessor.first.scopeCache, doubleViewSize, stride);
        renew(streamProcessor.second.scopeCache, doubleViewSize, stride);
    }
};

FxScope::FxScope(
        runtime::state_access const& state_access,
        runtime::fx::module_id const fx_mod_id)
    : FxModule{state_access, fx_mod_id}
    , m_impl{make_pimpl<Impl>(busType())}
{
    auto const parameters =
            runtime::parameters_map_view<parameter_key>(this->parameters());

    makeParameter(
            m_impl->mode,
            parameters.get<runtime::enum_parameter_id>(parameter_key::mode));

    makeParameter(
            m_impl->triggerSlope,
            parameters.get<runtime::enum_parameter_id>(
                    parameter_key::trigger_slope));

    makeParameter(
            m_impl->triggerLevel,
            parameters.get<runtime::float_parameter_id>(
                    parameter_key::trigger_level));

    makeParameter(
            m_impl->holdTime,
            parameters.get<runtime::float_parameter_id>(
                    parameter_key::hold_time));

    makeParameter(
            m_impl->waveformResolution,
            parameters.get<runtime::int_parameter_id>(
                    parameter_key::waveform_window_size));

    makeParameter(
            m_impl->scopeResolution,
            parameters.get<runtime::int_parameter_id>(
                    parameter_key::scope_window_size));

    makeParameter(
            m_impl->streamProcessor.first.active,
            parameters.get<runtime::bool_parameter_id>(
                    parameter_key::stream_a_active));

    makeParameter(
            m_impl->streamProcessor.second.active,
            parameters.get<runtime::bool_parameter_id>(
                    parameter_key::stream_b_active));

    makeParameter(
            m_impl->streamProcessor.first.channel,
            parameters.get<runtime::enum_parameter_id>(
                    parameter_key::channel_a));

    makeParameter(
            m_impl->streamProcessor.second.channel,
            parameters.get<runtime::enum_parameter_id>(
                    parameter_key::channel_b));

    makeParameter(
            m_impl->streamProcessor.first.gain,
            parameters.get<runtime::float_parameter_id>(parameter_key::gain_a));

    makeParameter(
            m_impl->streamProcessor.second.gain,
            parameters.get<runtime::float_parameter_id>(parameter_key::gain_b));

    makeParameter(
            m_impl->freeze,
            parameters.get<runtime::bool_parameter_id>(parameter_key::freeze));

    makeStream(
            m_impl->stream,
            streams().at(std::to_underlying(stream_key::input)));

    auto clear_fn = [this]() { clear(); };

    if (m_impl->busType == BusType::Mono)
    {
        QObject::connect(
                m_impl->stream.get(),
                &AudioStreamProvider::captured,
                this,
                [this](AudioStream captured) {
                    if (m_impl->freeze->value())
                    {
                        return;
                    }

                    BOOST_ASSERT(captured.num_channels() == 1);
                    m_impl->streamProcessor.first.processSamples(
                            captured.samples());

                    if (m_impl->modeEnum() != Mode::Free)
                    {
                        auto scope = m_impl->scopeGenerator.process(
                                0,
                                {m_impl->streamProcessor.first.scopeCache
                                         .cached()},
                                m_viewSize,
                                m_impl->triggerSlopeEnum(),
                                m_impl->triggerLevel->valueF(),
                                captured.num_frames(),
                                m_impl->holdTimeInFrames());

                        if (scope.size() == 1)
                        {
                            m_impl->streamProcessor.first.scope.update(
                                    scope[0]);
                        }
                    }
                });
    }
    else
    {
        QObject::connect(
                m_impl->stream.get(),
                &AudioStreamProvider::captured,
                this,
                [this](AudioStream captured) {
                    if (m_impl->freeze->value())
                    {
                        return;
                    }

                    auto stereo_captured = captured.channels_cast<2>();

                    m_impl->streamProcessor.first.processStereo(
                            stereo_captured);
                    m_impl->streamProcessor.second.processStereo(
                            stereo_captured);

                    if (m_impl->modeEnum() != Mode::Free)
                    {
                        auto scopeSamples = m_impl->scopeGenerator.process(
                                m_impl->modeEnum() == Mode::TriggerB,
                                {m_impl->streamProcessor.first.scopeCache
                                         .cached(),
                                 m_impl->streamProcessor.second.scopeCache
                                         .cached()},
                                m_viewSize,
                                m_impl->triggerSlopeEnum(),
                                m_impl->triggerLevel->value(),
                                captured.num_frames(),
                                m_impl->holdTimeInFrames());

                        if (scopeSamples.size() == 2)
                        {
                            m_impl->streamProcessor.first.scope.update(
                                    scopeSamples[0]);
                            m_impl->streamProcessor.second.scope.update(
                                    scopeSamples[1]);
                        }
                    }
                });

        QObject::connect(
                m_impl->streamProcessor.first.active.get(),
                &BoolParameter::valueChanged,
                this,
                clear_fn);

        QObject::connect(
                m_impl->streamProcessor.first.channel.get(),
                &EnumParameter::valueChanged,
                this,
                clear_fn);

        QObject::connect(
                m_impl->streamProcessor.second.active.get(),
                &BoolParameter::valueChanged,
                this,
                clear_fn);

        QObject::connect(
                m_impl->streamProcessor.second.channel.get(),
                &EnumParameter::valueChanged,
                this,
                clear_fn);
    }

    QObject::connect(
            m_impl->mode.get(),
            &EnumParameter::valueChanged,
            this,
            [this]() {
                m_impl->updateMode();
                clear();
            });

    QObject::connect(
            m_impl->waveformResolution.get(),
            &IntParameter::valueChanged,
            this,
            [this]() { m_impl->updateWaveformGenerator(); });

    QObject::connect(
            m_impl->scopeResolution.get(),
            &IntParameter::valueChanged,
            this,
            [this]() {
                m_impl->updateScopeSamplesCache(m_viewSize);
                clear();
            });

    QObject::connect(this, &FxScope::viewSizeChanged, this, [this]() {
        m_impl->updateWaveform(m_viewSize);
        m_impl->updateScopeSamplesCache(m_viewSize);
        clear();
    });
}

auto
FxScope::type() const noexcept -> FxModuleType
{
    return {.id = internal_id()};
}

auto
FxScope::mode() const noexcept -> EnumParameter*
{
    return m_impl->mode.get();
}

auto
FxScope::triggerSlope() const noexcept -> EnumParameter*
{
    return m_impl->triggerSlope.get();
}

auto
FxScope::triggerLevel() const noexcept -> FloatParameter*
{
    return m_impl->triggerLevel.get();
}

auto
FxScope::holdTime() const noexcept -> FloatParameter*
{
    return m_impl->holdTime.get();
}

auto
FxScope::waveformWindowSize() const noexcept -> IntParameter*
{
    return m_impl->waveformResolution.get();
}

auto
FxScope::scopeWindowSize() const noexcept -> IntParameter*
{
    return m_impl->scopeResolution.get();
}

auto
FxScope::activeA() const noexcept -> BoolParameter*
{
    return m_impl->streamProcessor.first.active.get();
}

auto
FxScope::activeB() const noexcept -> BoolParameter*
{
    return m_impl->streamProcessor.second.active.get();
}

auto
FxScope::channelA() const noexcept -> EnumParameter*
{
    return m_impl->streamProcessor.first.channel.get();
}

auto
FxScope::channelB() const noexcept -> EnumParameter*
{
    return m_impl->streamProcessor.second.channel.get();
}

auto
FxScope::gainA() const noexcept -> FloatParameter*
{
    return m_impl->streamProcessor.first.gain.get();
}

auto
FxScope::gainB() const noexcept -> FloatParameter*
{
    return m_impl->streamProcessor.second.gain.get();
}

auto
FxScope::freeze() const noexcept -> BoolParameter*
{
    return m_impl->freeze.get();
}

auto
FxScope::waveformA() const noexcept -> WaveformSlot*
{
    return &m_impl->streamProcessor.first.waveform;
}

auto
FxScope::waveformB() const noexcept -> WaveformSlot*
{
    return &m_impl->streamProcessor.second.waveform;
}

auto
FxScope::scopeA() const noexcept -> ScopeSlot*
{
    return &m_impl->streamProcessor.first.scope;
}

auto
FxScope::scopeB() const noexcept -> ScopeSlot*
{
    return &m_impl->streamProcessor.second.scope;
}

void
FxScope::clear()
{
    m_impl->streamProcessor.first.scope.clear();
    m_impl->streamProcessor.second.scope.clear();
    m_impl->scopeGenerator.clear();

    m_impl->streamProcessor.first.waveform.clear();
    m_impl->streamProcessor.second.waveform.clear();
    m_impl->streamProcessor.first.waveformGenerator.reset();
    m_impl->streamProcessor.second.waveformGenerator.reset();
}

void
FxScope::onSubscribe()
{
    auto sample_rate =
            observe_once(runtime::selectors::select_sample_rate)->current;
    m_impl->updateSampleRate(sample_rate);
    setSampleRate(sample_rate.as<double>());
}

} // namespace piejam::fx_modules::scope::gui
