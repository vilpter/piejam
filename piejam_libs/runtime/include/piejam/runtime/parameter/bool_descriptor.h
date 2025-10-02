// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/flags.h>

#include <piejam/boxed_string.h>

namespace piejam::runtime::parameter
{

constexpr auto
default_bool_to_string(bool x) -> std::string
{
    using namespace std::string_literals;
    return x ? "on"s : "off"s;
}

struct bool_descriptor
{
    using this_t = bool_descriptor;

    using value_type = bool;
    using value_to_string_fn = std::string (*)(value_type);

    auto operator==(this_t const&) const noexcept -> bool = default;

    boxed_string name;

    value_type default_value;

    value_to_string_fn value_to_string{&default_bool_to_string};

    M_PIEJAM_DEFINE_PARAMETER_FLAGS_MEMBER(this_t)
};

} // namespace piejam::runtime::parameter
