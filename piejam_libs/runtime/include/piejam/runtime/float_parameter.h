// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/descriptor.h>
#include <piejam/runtime/parameter/fwd.h>

#include <piejam/numeric/clamp.h>
#include <piejam/numeric/dB_convert.h>
#include <piejam/numeric/normalize.h>

#include <boost/assert.hpp>

#include <format>
#include <string_view>

namespace piejam::runtime
{

struct float_parameter_tag;
using float_parameter = parameter::descriptor<float_parameter_tag, float>;
using float_parameter_id = parameter::id_t<float_parameter>;

inline auto
default_float_to_string(float x) -> std::string
{
    return std::format("{:.2f}", x);
}

inline auto
default_float_to_dB_string(float x) -> std::string
{
    return std::format("{:.1f} dB", numeric::to_dB(x));
}

struct float_parameter_args
{
    std::string_view name;
    float default_value;
};

enum class float_parameter_range_kind
{
    linear,
    logarithmic,
};

struct float_parameter_range
{
    float min;
    float max;
    float_parameter_range_kind kind{};
};

inline auto
make_float_parameter(float_parameter_args args, float_parameter_range rng)
{
    BOOST_ASSERT(rng.min < rng.max);
    BOOST_ASSERT(rng.min <= args.default_value);
    BOOST_ASSERT(args.default_value <= rng.max);
    return float_parameter{
        .type = typeid(float),
        .name = box{std::string{args.name}},
        .default_value = args.default_value,
        .min = rng.min,
        .max = rng.max,
        .value_to_string = &default_float_to_string,
        .to_normalized =
            rng.kind == float_parameter_range_kind::logarithmic
                ? float_parameter::
                      to_normalized_f{[](auto const& d, float x) noexcept -> float {
                          return numeric::clamp(
                              numeric::to_normalized<float>(
                                  std::log(x),
                                  std::log(d.min),
                                  std::log(d.max)),
                              0.f,
                              1.f);
                      }}
                : float_parameter::
                      to_normalized_f{[](auto const& d, float x) noexcept -> float {
                          return numeric::clamp(
                              numeric::to_normalized<float>(x, d.min, d.max),
                              0.f,
                              1.f);
                      }},
        .from_normalized =
            rng.kind == float_parameter_range_kind::logarithmic
                ? float_parameter::
                      from_normalized_f{[](auto const& d, float n) noexcept -> float {
                          return numeric::clamp(
                              std::exp(
                                  numeric::from_normalized(
                                      n,
                                      std::log(d.min),
                                      std::log(d.max))),
                              d.min,
                              d.max);
                      }}
                : float_parameter::
                      from_normalized_f{[](auto const& d, float n) noexcept -> float {
                          return numeric::clamp(
                              numeric::from_normalized(n, d.min, d.max),
                              d.min,
                              d.max);
                      }},
    };
}

template <float Min, float Max, float_parameter_range_kind Kind>
    requires(Min < Max)
struct static_float_parameter_range
{
};

template <float Min, float Max>
using linear_float_parameter_range =
    static_float_parameter_range<Min, Max, float_parameter_range_kind::linear>;

template <float Min, float Max>
using logarithmic_float_parameter_range = static_float_parameter_range<
    Min,
    Max,
    float_parameter_range_kind::logarithmic>;

template <float Min, float Max>
using dB_float_parameter_range = static_float_parameter_range<
    numeric::from_dB(Min),
    numeric::from_dB(Max),
    float_parameter_range_kind::logarithmic>;

template <float Min, float Max, float_parameter_range_kind Kind>
auto
make_float_parameter(
    float_parameter_args args,
    static_float_parameter_range<Min, Max, Kind>)
{
    BOOST_ASSERT(Min <= args.default_value);
    BOOST_ASSERT(args.default_value <= Max);
    return float_parameter{
        .type = typeid(float),
        .name = box{std::string{args.name}},
        .default_value = args.default_value,
        .min = Min,
        .max = Max,
        .value_to_string = &default_float_to_string,
        .to_normalized = [](auto const&, float x) noexcept -> float {
            if constexpr (Kind == float_parameter_range_kind::linear)
            {
                return numeric::clamp(
                    numeric::to_normalized<float, float, Min, Max>(x),
                    0.f,
                    1.f);
            }
            else if constexpr (Kind == float_parameter_range_kind::logarithmic)
            {
                return numeric::clamp(
                    numeric::to_normalized<
                        float,
                        float,
                        std::log(Min),
                        std::log(Max)>(std::log(x)),
                    0.f,
                    1.f);
            }
        },
        .from_normalized = [](auto const&, float n) noexcept -> float {
            if constexpr (Kind == float_parameter_range_kind::linear)
            {
                return numeric::clamp(
                    numeric::from_normalized<float, float, Min, Max>(n),
                    Min,
                    Max);
            }
            else if constexpr (Kind == float_parameter_range_kind::logarithmic)
            {
                return numeric::clamp(
                    std::exp(
                        numeric::from_normalized<
                            float,
                            float,
                            std::log(Min),
                            std::log(Max)>(n)),
                    Min,
                    Max);
            }
        },
    };
}

} // namespace piejam::runtime
