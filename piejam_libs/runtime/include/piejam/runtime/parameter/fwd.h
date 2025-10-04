// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/enum.h>
#include <piejam/fwd.h>

namespace piejam::runtime::parameter
{

template <class Tag, class Value>
struct descriptor;

template <class Parameter>
using id_t = entity_id<Parameter>;

template <class>
struct value_type;

template <class Tag, class Value>
struct value_type<descriptor<Tag, Value>>
{
    using type = Value;
};

template <class Parameter>
using value_type_t = typename value_type<Parameter>::type;

template <class>
struct tagged_value;

template <class Tag, class Value>
struct tagged_value<descriptor<Tag, Value>>
{
    using tag_t = Tag;
    using value_type = Value;

    value_type value;

    [[nodiscard]]
    constexpr operator value_type() const noexcept
    {
        return value;
    }
};

class store;

using key = unsigned;

template <class Key>
concept enum_key = scoped_enum<Key, key>;

template <class ParameterId>
class map;

template <scoped_enum<key> E, class ParameterId>
class map_by;

template <scoped_enum<key> E, class ParameterId>
class map_view_by;

template <class Value>
struct assignment;

} // namespace piejam::runtime::parameter
