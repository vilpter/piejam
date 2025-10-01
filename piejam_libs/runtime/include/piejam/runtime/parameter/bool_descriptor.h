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
    using value_type = bool;
    using value_to_string_fn = std::string (*)(value_type);

    boxed_string name;

    value_type default_value;

    value_to_string_fn value_to_string{&default_bool_to_string};

    flags_set flags{};

    auto operator==(bool_descriptor const&) const noexcept -> bool = default;

    constexpr auto set_flags(flags_set flags) & -> bool_descriptor&
    {
        this->flags = flags;
        return *this;
    }

    [[nodiscard]]
    constexpr auto set_flags(flags_set flags) && -> bool_descriptor&&
    {
        this->flags = flags;
        return std::move(*this);
    }
};

} // namespace piejam::runtime::parameter
