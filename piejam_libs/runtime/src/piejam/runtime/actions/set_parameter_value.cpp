// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/actions/set_parameter_value.h>

#include <piejam/runtime/bool_parameter.h>
#include <piejam/runtime/enum_parameter.h>
#include <piejam/runtime/float_parameter.h>
#include <piejam/runtime/state.h>
#include <piejam/runtime/ui/thunk_action.h>

#include <piejam/functional/in_interval.h>

#include <boost/assert.hpp>

namespace piejam::runtime::actions
{

auto
parameter_value_is_in_range(bool_parameter const&, bool)
{
    return true;
}

template <class Parameter>
auto
parameter_value_is_in_range(
        Parameter const& param,
        parameter::value_type_t<Parameter> value)
{
    return in_closed(value, param.min, param.max);
}

template <class Parameter>
void
set_parameter_value<Parameter>::reduce(state& st) const
{
    auto& desc = st.params.at(id);
    BOOST_ASSERT(parameter_value_is_in_range(desc.param(), value));
    desc.set(value);

    if (desc.param().flags.test(parameter_flags::solo_state_affecting))
    {
        ++st.solo_state_update_count;
    }
}

template struct set_parameter_value<bool_parameter>;
template struct set_parameter_value<int_parameter>;
template struct set_parameter_value<float_parameter>;

auto
reset_parameter_to_default_value(parameter_id param_id) -> thunk_action
{
    return [=](auto&& get_state, auto&& dispatch) {
        std::visit(
                [&]<class P>(parameter::id_t<P> typed_param_id) {
                    state const& st = get_state();

                    dispatch(
                            set_parameter_value{
                                    typed_param_id,
                                    st.params.at(typed_param_id)
                                            .param()
                                            .default_value});
                },
                param_id);
    };
}

auto
set_float_parameter_normalized(
        float_parameter_id const param_id,
        float const norm_value) -> thunk_action
{
    return [=](auto const& get_state, auto const& dispatch) {
        state const& st = get_state();
        float_parameter const& param = st.params.at(param_id).param();
        dispatch(
                set_float_parameter{
                        param_id,
                        param.from_normalized(param, norm_value)});
    };
}

} // namespace piejam::runtime::actions
