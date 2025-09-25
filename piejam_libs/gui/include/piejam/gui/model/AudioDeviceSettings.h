// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SoundCardInfo.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>

#include <QVector>

namespace piejam::gui::model
{

class AudioDeviceSettings final : public SubscribableModel
{
    Q_OBJECT

    M_PIEJAM_GUI_PROPERTY(
            QVector<piejam::gui::model::SoundCardInfo>,
            soundCards,
            setSoundCards)
    M_PIEJAM_GUI_PROPERTY(
            int,
            selectedSoundCardIndex,
            setSelectedSoundCardIndex)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::StringList*, sampleRates)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::StringList*, periodSizes)
    M_PIEJAM_GUI_PROPERTY(double, bufferLatency, setBufferLatency)

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
