// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/actions/fx_chain_actions.h>

#include <piejam/bool.h>
#include <piejam/runtime/state.h>

namespace piejam::runtime::actions
{

void
toggle_focused_fx_module_bypass::reduce(state& st) const
{
    toggle_bool_in_place(
        st.fx_state.modules.lock().at(st.focused_fx_mod_id).bypassed);
}

void
focus_fx_module::reduce(state& st) const
{
    st.focused_fx_chain_id = fx_chain_id;
    st.focused_fx_mod_id = fx_mod_id;
}

void
show_fx_module::reduce(state& st) const
{
    st.root_view_mode = runtime::root_view_mode::fx_module;
}

} // namespace piejam::runtime::actions
