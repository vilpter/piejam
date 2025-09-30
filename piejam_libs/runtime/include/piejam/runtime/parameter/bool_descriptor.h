// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/boxed_string.h>

namespace piejam::runtime::parameter
{

inline auto
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

    bool midi_assignable{true};
    bool audio_graph_affecting{false};
};

} // namespace piejam::runtime::parameter
