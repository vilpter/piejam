// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/fwd.h>
#include <piejam/runtime/persistence/session.h>
#include <piejam/runtime/ui/cloneable_action.h>

#include <piejam/box.h>

namespace piejam::runtime::actions
{

struct apply_session final
    : ui::cloneable_action<apply_session, reducible_action>
{
    box<persistence::session> session;

    void reduce(state&) const override;
};

} // namespace piejam::runtime::actions
