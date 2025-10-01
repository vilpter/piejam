// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/flags.h>

#include <piejam/boxed_string.h>

#include <format>

namespace piejam::runtime::parameter
{

inline auto
default_float_to_string(float x) -> std::string
{
    return std::format("{:.2f}", x);
}

struct float_descriptor
{
    using value_type = float;
    using value_to_string_fn = std::string (*)(value_type);
    using to_normalized_f = value_type (*)(float_descriptor const&, value_type);
    using from_normalized_f =
            value_type (*)(float_descriptor const&, value_type);

    boxed_string name;

    value_type default_value{};

    value_type min{};
    value_type max{1.f};

    bool bipolar{};

    value_to_string_fn value_to_string{&default_float_to_string};

    to_normalized_f to_normalized{[](auto const&, value_type x) { return x; }};
    from_normalized_f from_normalized{
            [](auto const&, value_type x) { return x; }};

    flags_set flags{};

    auto operator==(float_descriptor const&) const noexcept -> bool = default;

    constexpr auto set_flags(flags_set flags) & -> float_descriptor&
    {
        this->flags = flags;
        return *this;
    }

    [[nodiscard]]
    constexpr auto set_flags(flags_set flags) && -> float_descriptor&&
    {
        this->flags = flags;
        return std::move(*this);
    }
};

} // namespace piejam::runtime::parameter
