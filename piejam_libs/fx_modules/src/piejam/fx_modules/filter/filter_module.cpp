// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/fx_modules/filter/filter_module.h>

#include <piejam/fx_modules/filter/filter_internal_id.h>

#include <piejam/audio/multichannel_buffer.h>
#include <piejam/entity_map.h>
#include <piejam/runtime/enum_parameter.h>
#include <piejam/runtime/fx/module.h>
#include <piejam/runtime/parameter/float_descriptor.h>
#include <piejam/runtime/parameter/float_normalize.h>
#include <piejam/runtime/parameter/int_descriptor.h>
#include <piejam/runtime/parameter_factory.h>
#include <piejam/runtime/parameters_map.h>

#include <format>

namespace piejam::fx_modules::filter
{

namespace
{

auto
to_type_string(int const n) -> std::string
{
    switch (n)
    {
        case std::to_underlying(type::lp2):
            return "LP2";
        case std::to_underlying(type::lp4):
            return "LP4";
        case std::to_underlying(type::bp2):
            return "BP2";
        case std::to_underlying(type::bp4):
            return "BP4";
        case std::to_underlying(type::hp2):
            return "HP2";
        case std::to_underlying(type::hp4):
            return "HP4";
        case std::to_underlying(type::br):
            return "BR";

        default:
            return "Pass";
    }
}

auto
to_cutoff_string(float const f)
{
    return f < 1000.f ? std::format("{:.2f} Hz", f)
                      : std::format("{:.2f} kHz", f / 1000.f);
}

auto
to_resonance_string(float const r)
{
    auto p = r * 100;
    return p < 10.f ? std::format("{:.2f}%", p)
                    : (p < 100.f ? std::format("{:.1f}%", p)
                                 : std::format("{:.0f}%", p));
}

} // namespace

auto
make_module(runtime::internal_fx_module_factory_args const& args)
        -> runtime::fx::module
{
    using namespace std::string_literals;

    runtime::parameter_factory params_factory{args.params};

    return runtime::fx::module{
            .fx_instance_id = internal_id(),
            .name = box("Filter"s),
            .bus_type = args.bus_type,
            .parameters =
                    box(runtime::parameters_map_by<parameter_key>{
                            {parameter_key::type,
                             params_factory.make_parameter(
                                     runtime::enum_parameter<type>(
                                             "Type"s,
                                             &to_type_string))},
                            {parameter_key::cutoff,
                             params_factory.make_parameter(
                                     runtime::float_parameter{
                                             .name = box("Cutoff"s),
                                             .default_value = 440.f,
                                             .min = 10.f,
                                             .max = 20000.f,
                                             .value_to_string =
                                                     &to_cutoff_string,
                                             .to_normalized =
                                                     &runtime::parameter::
                                                             to_normalized_log,
                                             .from_normalized =
                                                     &runtime::parameter::
                                                             from_normalized_log})},
                            {parameter_key::resonance,
                             params_factory.make_parameter(
                                     runtime::float_parameter{
                                             .name = box("Resonance"s),
                                             .default_value = 0.f,
                                             .min = 0.f,
                                             .max = 1.f,
                                             .value_to_string =
                                                     &to_resonance_string,
                                             .to_normalized =
                                                     &runtime::parameter::
                                                             to_normalized_linear,
                                             .from_normalized =
                                                     &runtime::parameter::
                                                             from_normalized_linear})}}
                                .as_base()),
            .streams =
                    box(runtime::fx::module_streams{
                            {std::to_underlying(stream_key::in_out),
                             make_stream(
                                     args.streams,
                                     num_channels(args.bus_type) * 2)},
                    })};
}

} // namespace piejam::fx_modules::filter
