// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/MixerChannelPerform.h>

#include <piejam/gui/model/AudioStreamProvider.h>
#include <piejam/gui/model/BoolParameter.h>
#include <piejam/gui/model/FloatParameter.h>
#include <piejam/gui/model/StereoLevel.h>

#include <piejam/audio/pair.h>
#include <piejam/functional/operators.h>
#include <piejam/numeric/flush_to_zero_if.h>
#include <piejam/numeric/mipp_iterator.h>
#include <piejam/numeric/simd/math.h>
#include <piejam/numeric/simd/rms.h>
#include <piejam/renew.h>
#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

namespace
{

template <std::floating_point T>
auto
calcPeakLevel(
        std::span<T const> const samples,
        T const currentLevel,
        audio::sample_rate const sr) -> T
{
    T const blockPeak = mipp::hmax(
            std::transform_reduce(
                    numeric::mipp_begin(samples),
                    numeric::mipp_end(samples),
                    mipp::Reg<T>(T{}),
                    numeric::simd::max,
                    numeric::simd::abs));

    T const dt = static_cast<T>(samples.size()) / sr.as<T>();
    constexpr T tau = T{0.2}; // 200ms
    T const coeff = std::exp(-dt / tau);
    T const newLevel = std::max(blockPeak, currentLevel * coeff);

    return numeric::flush_to_zero_if(newLevel, less(T{1e-3})); // -60dB
}

template <std::floating_point T>
auto
calcRmsLevel(
        std::span<T const> const samples,
        T const currentLevel,
        audio::sample_rate const sr) -> T
{
    T const blockRms = numeric::simd::rms(samples);

    T const dt = static_cast<T>(samples.size()) / sr.as<T>();

    constexpr T attack = T{0.06}; // 60ms
    constexpr T release = T{0.4}; // 400ms;

    T const tau = blockRms > currentLevel ? attack : release;
    T const coeff = std::exp(-dt / tau);
    T const newLevel = coeff * currentLevel + (1 - coeff) * blockRms;

    return numeric::flush_to_zero_if(newLevel, less(T{1e-8})); // -160dB
}

} // namespace

struct MixerChannelPerform::Impl
{
    static constexpr audio::sample_rate default_sample_rate{48000u};
    audio::sample_rate sample_rate{default_sample_rate};

    StereoLevel peakLevel;
    StereoLevel rmsLevel;
    std::unique_ptr<AudioStreamProvider> outStream;

    std::unique_ptr<FloatParameter> volume;
    std::unique_ptr<FloatParameter> panBalance;
    std::unique_ptr<BoolParameter> record;
    std::unique_ptr<BoolParameter> solo;
    std::unique_ptr<BoolParameter> mute;

    void updateSampleRate(audio::sample_rate sr)
    {
        if (sr.valid() && sample_rate != sr)
        {
            sample_rate = sr;
        }
    }

    template <audio::pair_channel C>
    void updatePeakLevel(std::span<float const> ch)
    {
        peakLevel.setLevel<C>(static_cast<double>(calcPeakLevel<float>(
                ch,
                static_cast<float>(peakLevel.level<C>()),
                sample_rate)));
    }

    template <audio::pair_channel C>
    void updateRmsLevel(std::span<float const> ch)
    {
        rmsLevel.setLevel<C>(static_cast<double>(calcRmsLevel<float>(
                ch,
                static_cast<float>(rmsLevel.level<C>()),
                sample_rate)));
    }

    void resetLevelMeter()
    {
        peakLevel.reset();
        rmsLevel.reset();
    }
};

MixerChannelPerform::MixerChannelPerform(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id)
    : MixerChannel{state_access, id}
    , m_impl{make_pimpl<Impl>()}
{
    makeStream(
            m_impl->outStream,
            observe_once(
                    runtime::selectors::make_mixer_channel_out_stream_selector(
                            id)));

    QObject::connect(
            m_impl->outStream.get(),
            &AudioStreamProvider::captured,
            this,
            [this](AudioStream captured) {
                auto captured_stereo = captured.channels_cast<2>();

                m_impl->updatePeakLevel<audio::pair_channel::left>(
                        captured_stereo.channels()[0]);
                m_impl->updatePeakLevel<audio::pair_channel::right>(
                        captured_stereo.channels()[1]);

                m_impl->updateRmsLevel<audio::pair_channel::left>(
                        captured_stereo.channels()[0]);
                m_impl->updateRmsLevel<audio::pair_channel::right>(
                        captured_stereo.channels()[1]);
            });

    makeParameter(
            m_impl->volume,
            observe_once(
                    runtime::selectors::
                            make_mixer_channel_volume_parameter_selector(id)));

    makeParameter(
            m_impl->panBalance,
            observe_once(
                    runtime::selectors::
                            make_mixer_channel_pan_balance_parameter_selector(
                                    id)));

    makeParameter(
            m_impl->record,
            observe_once(
                    runtime::selectors::
                            make_mixer_channel_record_parameter_selector(id)));

    makeParameter(
            m_impl->solo,
            observe_once(
                    runtime::selectors::
                            make_mixer_channel_solo_parameter_selector(id)));

    makeParameter(
            m_impl->mute,
            observe_once(
                    runtime::selectors::
                            make_mixer_channel_mute_parameter_selector(id)));
}

auto
MixerChannelPerform::peakLevel() const noexcept -> StereoLevel*
{
    return &m_impl->peakLevel;
}

auto
MixerChannelPerform::rmsLevel() const noexcept -> StereoLevel*
{
    return &m_impl->rmsLevel;
}

auto
MixerChannelPerform::volume() const noexcept -> FloatParameter*
{
    return m_impl->volume.get();
}

auto
MixerChannelPerform::panBalance() const noexcept -> FloatParameter*
{
    return m_impl->panBalance.get();
}

auto
MixerChannelPerform::record() const noexcept -> BoolParameter*
{
    return m_impl->record.get();
}

auto
MixerChannelPerform::solo() const noexcept -> BoolParameter*
{
    return m_impl->solo.get();
}

auto
MixerChannelPerform::mute() const noexcept -> BoolParameter*
{
    return m_impl->mute.get();
}

void
MixerChannelPerform::onSubscribe()
{
    m_impl->resetLevelMeter();

    MixerChannel::onSubscribe();

    m_impl->updateSampleRate(
            observe_once(runtime::selectors::select_sample_rate)->current);

    observe(runtime::selectors::make_muted_by_solo_selector(channel_id()),
            [this](bool x) { setMutedBySolo(x); });
}

} // namespace piejam::gui::model
