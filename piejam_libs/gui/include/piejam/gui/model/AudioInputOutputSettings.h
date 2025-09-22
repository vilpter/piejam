// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/gui/PropertyMacros.h>
#include <piejam/gui/model/GenericListModel.h>
#include <piejam/gui/model/SubscribableModel.h>
#include <piejam/gui/model/fwd.h>

#include <piejam/io_direction.h>
#include <piejam/pimpl.h>

#include <QStringList>

namespace piejam::gui::model
{

class AudioInputOutputSettings : public SubscribableModel
{
    Q_OBJECT

    M_PIEJAM_GUI_PROPERTY(QStringList, channels, setChannels)
    M_PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, deviceConfigs)

public:
    AudioInputOutputSettings(
            runtime::store_dispatch,
            runtime::subscriber&,
            io_direction);

    Q_INVOKABLE void addMonoDevice();
    Q_INVOKABLE void addStereoDevice();

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
