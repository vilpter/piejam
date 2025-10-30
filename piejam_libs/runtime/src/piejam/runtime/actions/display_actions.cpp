// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/actions/display_actions.h>

#include <piejam/runtime/state.h>

namespace piejam::runtime::actions
{

void
set_display_rotation::reduce(state& st) const
{
    st.display_rotation = display_rotation;
}

} // namespace piejam::runtime::actions
