// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/AudioDeviceSettings.h>

#include <piejam/gui/model/StringList.h>

#include <piejam/algorithm/index_of.h>
#include <piejam/audio/sound_card_descriptor.h>
#include <piejam/redux/store.h>
#include <piejam/redux/subscriptions_manager.h>
#include <piejam/runtime/actions/initiate_sound_card_selection.h>
#include <piejam/runtime/actions/scan_for_sound_cards.h>
#include <piejam/runtime/actions/select_period_count.h>
#include <piejam/runtime/actions/select_period_size.h>
#include <piejam/runtime/actions/select_sample_rate.h>
#include <piejam/runtime/selectors.h>
#include <piejam/runtime/ui/thunk_action.h>

#include <boost/hof/lift.hpp>

#include <algorithm>
#include <ranges>

namespace piejam::gui::model
{

namespace
{

template <class Range, class F>
auto
to_QStringList(Range const& l, F&& f) -> QStringList
{
    QStringList result;
    std::ranges::transform(l, std::back_inserter(result), std::forward<F>(f));
    return result;
}

auto const QString_from_number = BOOST_HOF_LIFT(QString::number);

} // namespace

struct AudioDeviceSettings::Impl
{
    StringList input_sound_cards;
    StringList output_sound_cards;
    StringList sample_rates;
    StringList period_sizes;
};

AudioDeviceSettings::AudioDeviceSettings(
        runtime::store_dispatch store_dispatch,
        runtime::subscriber& state_change_subscriber)
    : Subscribable(store_dispatch, state_change_subscriber)
    , m_impl{make_pimpl<Impl>()}
{
}

auto
AudioDeviceSettings::soundCards() const noexcept -> soundCards_property_t
{
    return &m_impl->input_sound_cards;
}

auto
AudioDeviceSettings::sampleRates() const noexcept -> sampleRates_property_t
{
    return &m_impl->sample_rates;
}

auto
AudioDeviceSettings::periodSizes() const noexcept -> periodSizes_property_t
{
    return &m_impl->period_sizes;
}

void
AudioDeviceSettings::onSubscribe()
{
    namespace selectors = runtime::selectors;

    observe(selectors::select_sound_card,
            [this](selectors::sound_card_choice const& choice) {
                soundCards()->setElements(to_QStringList(
                        choice.available,
                        &QString::fromStdString));
                soundCards()->setFocused(static_cast<int>(choice.current));
            });

    observe(selectors::select_sample_rate,
            [this](selectors::sample_rate_choice const& sample_rate) {
                auto const index = algorithm::index_of(
                        sample_rate.available,
                        sample_rate.current);

                sampleRates()->setElements(to_QStringList(
                        sample_rate.available |
                                std::views::transform(
                                        &audio::sample_rate::value),
                        QString_from_number));
                sampleRates()->setFocused(static_cast<int>(index));
            });

    observe(selectors::select_period_size,
            [this](selectors::period_size_choice const& period_size) {
                auto const index = algorithm::index_of(
                        period_size.available,
                        period_size.current);

                periodSizes()->setElements(to_QStringList(
                        period_size.available |
                                std::views::transform(
                                        &audio::period_size::value),
                        QString_from_number));
                periodSizes()->setFocused(static_cast<int>(index));
            });

    observe(selectors::select_buffer_latency, [this](float const x) {
        setBufferLatency(static_cast<double>(x));
    });

    requestUpdates(std::chrono::seconds{1}, [this]() {
        refreshSoundCardLists();
    });
}

void
AudioDeviceSettings::refreshSoundCardLists()
{
    dispatch(runtime::actions::scan_for_sound_cards());
}

void
AudioDeviceSettings::selectSoundCard(unsigned const index)
{
    runtime::actions::initiate_sound_card_selection action;
    action.index = index;
    dispatch(action);
}

void
AudioDeviceSettings::selectSampleRate(unsigned const index)
{
    runtime::actions::select_sample_rate action;
    action.index = index;
    dispatch(action);
}

void
AudioDeviceSettings::selectPeriodSize(unsigned const index)
{
    runtime::actions::select_period_size action;
    action.index = index;
    dispatch(action);
}

void
AudioDeviceSettings::selectPeriodCount(unsigned const index)
{
    runtime::actions::select_period_count action;
    action.index = index;
    dispatch(action);
}

} // namespace piejam::gui::model
