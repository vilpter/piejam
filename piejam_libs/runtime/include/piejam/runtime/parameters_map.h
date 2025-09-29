// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameters.h>

#include <piejam/entity_id.h>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/hof/unpack.hpp>

#include <ranges>

namespace piejam::runtime
{

template <class Key>
concept parameter_enum_key =
        std::is_scoped_enum_v<Key> &&
        std::is_same_v<parameter::key, std::underlying_type_t<Key>>;

template <parameter_enum_key Key>
class parameters_map_by : public parameters_map
{
public:
    parameters_map_by() = default;

    parameters_map_by(
            std::initializer_list<std::pair<Key, parameter_id>> params)
        : parameters_map{
                  params |
                  std::views::transform(
                          boost::hof::unpack([](Key key, parameter_id id) {
                              return std::pair{std::to_underlying(key), id};
                          })) |
                  std::ranges::to<parameters_map>()}
    {
    }

    auto emplace(Key key, parameter_id id) -> parameters_map_by&
    {
        parameters_map::emplace(std::to_underlying(key), id);
        return *this;
    }

    template <class ParameterId>
    auto get(Key key) const -> ParameterId
    {
        auto it = parameters_map::find(std::to_underlying(key));
        BOOST_ASSERT(it != parameters_map::end());
        return std::get<ParameterId>(it->second);
    }

    auto as_base() && -> parameters_map&&
    {
        return std::forward<parameters_map>(*this);
    }
};

template <parameter_enum_key Key>
class parameters_map_view
{
public:
    explicit parameters_map_view(parameters_map const& map)
        : m_map{map}
    {
    }

    template <class ParameterId>
    auto get(Key key) const -> ParameterId
    {
        auto it = m_map.find(std::to_underlying(key));
        BOOST_ASSERT(it != m_map.end());
        return std::get<ParameterId>(it->second);
    }

private:
    parameters_map const& m_map;
};

} // namespace piejam::runtime
