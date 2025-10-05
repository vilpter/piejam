// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/processors/midi_to_parameter_processor.h>

#include <piejam/runtime/bool_parameter.h>
#include <piejam/runtime/enum_parameter.h>
#include <piejam/runtime/float_parameter.h>
#include <piejam/runtime/int_parameter.h>

#include <piejam/audio/engine/event_converter_processor.h>
#include <piejam/audio/engine/event_input_buffers.h>
#include <piejam/audio/engine/event_output_buffers.h>
#include <piejam/audio/engine/event_port.h>
#include <piejam/audio/engine/named_processor.h>
#include <piejam/audio/engine/verify_process_context.h>
#include <piejam/midi/event.h>
#include <piejam/range/iota.h>

#include <boost/assert.hpp>

#include <algorithm>
#include <array>
#include <memory>

namespace piejam::runtime::processors
{

static constexpr std::array s_input_names{std::string_view("cc in")};

template <class Parameter>
static auto
make_cc_to_value_lut(Parameter const& param)
{
    using value_type = typename Parameter::value_type;
    std::array<value_type, 128> result;

    std::ranges::transform(
        range::iota(std::size_t{128}),
        result.begin(),
        [&param](std::size_t const cc_value) -> value_type {
            return param.from_normalized(param, cc_value / 127.f);
        });

    return result;
}

template <class Parameter>
auto
make_midi_cc_to_parameter_processor(Parameter const& param)
    -> std::unique_ptr<audio::engine::processor>
{
    using value_type = typename Parameter::value_type;
    static constexpr std::array s_output_names{std::string_view("out")};
    return audio::engine::make_event_converter_processor(
        [lut = make_cc_to_value_lut(param)](midi::cc_event const& cc_ev)
            -> value_type { return lut[cc_ev.value]; },
        s_input_names,
        s_output_names,
        "cc_to_float");
}

template auto make_midi_cc_to_parameter_processor(bool_parameter const&)
    -> std::unique_ptr<audio::engine::processor>;
template auto make_midi_cc_to_parameter_processor(enum_parameter const&)
    -> std::unique_ptr<audio::engine::processor>;
template auto make_midi_cc_to_parameter_processor(float_parameter const&)
    -> std::unique_ptr<audio::engine::processor>;
template auto make_midi_cc_to_parameter_processor(int_parameter const&)
    -> std::unique_ptr<audio::engine::processor>;

} // namespace piejam::runtime::processors
