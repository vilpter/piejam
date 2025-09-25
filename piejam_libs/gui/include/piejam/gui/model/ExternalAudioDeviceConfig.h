// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/pimpl.h>
#include <piejam/runtime/external_audio_fwd.h>

namespace piejam::gui::model
{

class ExternalAudioDeviceConfig final : public SubscribableModel
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(piejam::gui::model::String*, name)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(bool, mono)
    M_PIEJAM_GUI_PROPERTY(int, monoChannel, setMonoChannel)
    M_PIEJAM_GUI_PROPERTY(int, stereoLeftChannel, setStereoLeftChannel)
    M_PIEJAM_GUI_PROPERTY(int, stereoRightChannel, setStereoRightChannel)

public:
    ExternalAudioDeviceConfig(
            runtime::state_access const&,
            runtime::external_audio::device_id);

    Q_INVOKABLE void changeMonoChannel(unsigned);
    Q_INVOKABLE void changeStereoLeftChannel(unsigned);
    Q_INVOKABLE void changeStereoRightChannel(unsigned);
    Q_INVOKABLE void remove();

private:
    void onSubscribe() override;

    runtime::external_audio::device_id m_device_id;
    pimpl<String> m_string;
    bool m_mono;
};

} // namespace piejam::gui::model
