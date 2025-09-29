// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/boxed_string.h>

#include <format>

namespace piejam::runtime::parameter
{

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

    value_type bipolar{};

    value_to_string_fn value_to_string{
            [](value_type x) { return std::format("{:.2f}", x); }};

    to_normalized_f to_normalized{[](auto const&, value_type x) { return x; }};
    from_normalized_f from_normalized{
            [](auto const&, value_type x) { return x; }};

    bool midi_assignable{true};
    bool audio_graph_affecting{false};
};

} // namespace piejam::runtime::parameter
