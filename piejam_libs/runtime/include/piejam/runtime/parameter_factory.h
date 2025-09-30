// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/fwd.h>
#include <piejam/runtime/parameters_store.h>

#include <boost/container/flat_map.hpp>

#include <tuple>

namespace piejam::runtime
{

template <template <class> class... Value>
class parameter_factory
{
public:
    explicit parameter_factory(
            parameters_store& params,
            parameter::store<Value>&... aux_param_maps)
        : m_params{params}
        , m_aux_param_maps{std::forward_as_tuple(aux_param_maps...)}
    {
    }

    template <class P, class... Vs>
    auto make_parameter(P&& param, Vs&&... aux_value) const
    {
        auto param_id = parameter::id_t<P>::generate();
        m_params.emplace(param_id, std::forward<P>(param));
        std::apply(
                [&](auto&&... aux_map) {
                    (aux_map.emplace(
                             param_id,
                             Value<P>{std::forward<Vs>(aux_value)}),
                     ...);
                },
                m_aux_param_maps);
        return param_id;
    }

private:
    parameters_store& m_params;
    std::tuple<parameter::store<Value>&...> m_aux_param_maps;
};

template <template <class> class... Value>
parameter_factory(parameters_store&, parameter::store<Value>&...)
        -> parameter_factory<Value...>;

} // namespace piejam::runtime
