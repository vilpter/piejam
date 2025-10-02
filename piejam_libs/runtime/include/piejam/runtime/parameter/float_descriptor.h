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
    using this_t = float_descriptor;

    using value_type = float;
    using value_to_string_fn = std::string (*)(value_type);
    using normalized_type = float;
    using to_normalized_f = normalized_type (*)(this_t const&, value_type);
    using from_normalized_f = value_type (*)(this_t const&, normalized_type);

    auto operator==(this_t const&) const noexcept -> bool = default;

    boxed_string name;

    value_type default_value{};

    value_type min{};
    value_type max{1.f};

    value_to_string_fn value_to_string{&default_float_to_string};

    to_normalized_f to_normalized{
            [](float_descriptor const&, value_type x) { return x; }};
    from_normalized_f from_normalized{
            [](float_descriptor const&, normalized_type x) { return x; }};

    M_PIEJAM_DEFINE_PARAMETER_FLAGS_MEMBER(this_t)
};

} // namespace piejam::runtime::parameter
