// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/int_descriptor.h>
#include <piejam/runtime/parameters.h>

#include <utility>

namespace piejam::runtime
{

template <class T>
    requires std::is_scoped_enum_v<T> &&
             std::is_convertible_v<std::underlying_type_t<T>, int>
auto
enum_parameter(
        std::string name,
        int_parameter::value_to_string_fn value_to_string,
        T _default = T::_default,
        T _min = T::_min,
        T _max = T::_max) -> int_parameter
{
    return int_parameter{
            .name = box(std::move(name)),
            .default_value = std::to_underlying(_default),
            .min = std::to_underlying(_min),
            .max = std::to_underlying(_max),
            .value_to_string = value_to_string,
    };
}

} // namespace piejam::runtime
