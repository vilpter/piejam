// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>

class QAbstractListModel;

namespace piejam::gui::model
{

class AudioDeviceSettings final : public SubscribableModel
{
    Q_OBJECT

    PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, soundCards)
    PIEJAM_GUI_PROPERTY(int, selectedSoundCardIndex, setSelectedSoundCardIndex)
    PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, sampleRates)
    PIEJAM_GUI_PROPERTY(int, selectedSampleRate, setSelectedSampleRate)
    PIEJAM_GUI_PROPERTY(int, periodSizesCount, setPeriodSizesCount)
    PIEJAM_GUI_PROPERTY(
        int,
        selectedPeriodSizeIndex,
        setSelectedPeriodSizeIndex)
    PIEJAM_GUI_PROPERTY(int, selectedPeriodSize, setSelectedPeriodSize)
    PIEJAM_GUI_PROPERTY(double, bufferLatency, setBufferLatency)

public:
    explicit AudioDeviceSettings(runtime::state_access const&);

    Q_INVOKABLE void selectSoundCard(unsigned index);
    Q_INVOKABLE void selectSampleRate(unsigned index);
    Q_INVOKABLE void selectPeriodSize(unsigned index);

private:
    void onSubscribe() override;

    void refreshSoundCardLists();

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
