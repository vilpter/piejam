// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/fwd.h>

namespace piejam::runtime::actions
{

auto scan_for_sound_cards() -> thunk_action;

} // namespace piejam::runtime::actions
