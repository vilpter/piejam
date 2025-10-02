// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/flags.h>

#include <piejam/boxed_string.h>

namespace piejam::runtime::parameter
{

inline auto
default_int_to_string(int x) -> std::string
{
    return std::to_string(x);
}

struct int_descriptor
{
    using this_t = int_descriptor;

    using value_type = int;
    using value_to_string_fn = std::string (*)(value_type);

    auto operator==(this_t const&) const noexcept -> bool = default;

    boxed_string name;

    value_type default_value;

    value_type min;
    value_type max;

    value_to_string_fn value_to_string{&default_int_to_string};

    M_PIEJAM_DEFINE_PARAMETER_FLAGS_MEMBER(this_t)
};

} // namespace piejam::runtime::parameter
