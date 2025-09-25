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

class MidiInputSettings final : public SubscribableModel
{
    Q_OBJECT

    M_PIEJAM_GUI_CONSTANT_PROPERTY(QAbstractListModel*, devices)

public:
    MidiInputSettings(runtime::state_access const&);

private:
    void onSubscribe() override;

    struct Impl;
    pimpl<Impl> m_impl;
};

} // namespace piejam::gui::model
