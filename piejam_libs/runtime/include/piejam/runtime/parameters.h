// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/fwd.h>

#include <boost/container/container_fwd.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/map.hpp>

#include <variant>

namespace piejam::runtime
{

struct midi_assignment;

using float_parameter = parameter::float_descriptor;
using float_parameter_id = parameter::id_t<float_parameter>;

using bool_parameter = parameter::bool_descriptor;
using bool_parameter_id = parameter::id_t<bool_parameter>;

using int_parameter = parameter::int_descriptor;
using int_parameter_id = parameter::id_t<int_parameter>;

using enum_parameter_id = int_parameter_id;

// Store the value type here, so we don't have to include the descriptor
// headers.
using parameters_fwd_t = boost::mp11::mp_list<
        boost::mp11::mp_list<bool_parameter, bool>,
        boost::mp11::mp_list<float_parameter, float>,
        boost::mp11::mp_list<int_parameter, int>>;

using parameter_ids_t = boost::mp11::mp_transform<
        parameter::id_t,
        boost::mp11::mp_map_keys<parameters_fwd_t>>;

template <class Parameter>
using parameter_value_type_t = boost::mp11::mp_second<
        boost::mp11::mp_map_find<parameters_fwd_t, Parameter>>;

using parameter_id = boost::mp11::mp_rename<parameter_ids_t, std::variant>;

using parameter_value = boost::mp11::mp_rename<
        boost::mp11::mp_transform<boost::mp11::mp_second, parameters_fwd_t>,
        std::variant>;

template <class ParamId>
using is_persistable_parameter =
        boost::mp11::mp_contains<parameter_ids_t, ParamId>;

template <class ParamId>
constexpr bool is_persistable_parameter_v =
        is_persistable_parameter<ParamId>::value;

using parameters_map = boost::container::flat_map<parameter::key, parameter_id>;

template <class Key>
concept parameter_enum_key =
        std::is_scoped_enum_v<Key> &&
        std::is_same_v<parameter::key, std::underlying_type_t<Key>>;

template <parameter_enum_key Key>
class parameters_map_by;

using parameter_value_assignment = parameter::assignment<parameter_value>;
using parameter_midi_assignment = parameter::assignment<midi_assignment>;

template <class Parameter>
struct parameter_store_slot;
using parameters_store = parameter::store<parameter_store_slot>;

} // namespace piejam::runtime
