// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/runtime/parameter/fwd.h>

#include <piejam/lean_map_facade.h>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>
#include <variant>

namespace piejam::runtime::parameter
{

template <class>
class map;

template <class... ParameterId>
class map<std::variant<ParameterId...>>
{
    using parameter_id_t = std::variant<ParameterId...>;
    using map_t =
            lean_map_facade<boost::container::flat_map<key, parameter_id_t>>;

public:
    map() noexcept = default;

    map(std::initializer_list<std::pair<key, parameter_id_t>> params)
        : m_map{std::move(params)}
    {
    }

    [[nodiscard]]
    auto size() const noexcept -> std::size_t
    {
        return m_map.size();
    }

    [[nodiscard]]
    auto begin() const noexcept
    {
        return m_map.begin();
    }

    [[nodiscard]]
    auto end() const noexcept
    {
        return m_map.end();
    }

    auto emplace(key key, parameter_id_t id) -> map&
    {
        BOOST_VERIFY(m_map.emplace(key, id).second);
        return *this;
    }

    auto find(key key) const noexcept
    {
        return m_map.find(key);
    }

    auto at(key key) const -> parameter_id_t
    {
        return m_map.at(key);
    }

    template <class P>
    auto get(key key) const -> P
    {
        return std::get<P>(m_map.at(key));
    }

    template <scoped_enum<key> E>
    auto view_by() const noexcept -> map_view_by<E, parameter_id_t>;

private:
    map_t m_map;
};

template <scoped_enum<key> E, class ParameterId>
class map_by : public map<ParameterId>
{
    using base_t = map<ParameterId>;

public:
    map_by() noexcept = default;

    map_by(std::initializer_list<std::pair<E, ParameterId>> params)
    {
        for (auto [key, id] : params)
        {
            base_t::emplace(std::to_underlying(key), id);
        }
    }

    auto emplace(E key, ParameterId id) -> map_by&
    {
        base_t::emplace(std::to_underlying(key), id);
        return *this;
    }

    auto as_base() && -> map<ParameterId>&&
    {
        return std::forward<map<ParameterId>>(*this);
    }
};

template <scoped_enum<key> E, class ParameterId>
class map_view_by
{
public:
    explicit map_view_by(map<ParameterId> const& m) noexcept
        : m_map{m}
    {
    }

    template <class P>
    auto get(E key) const -> P
    {
        return m_map.template get<P>(std::to_underlying(key));
    }

private:
    map<ParameterId> const& m_map;
};

template <class... ParameterId>
template <scoped_enum<key> E>
auto
map<std::variant<ParameterId...>>::view_by() const noexcept
        -> map_view_by<E, std::variant<ParameterId...>>
{
    return map_view_by<E, std::variant<ParameterId...>>{*this};
}

} // namespace piejam::runtime::parameter
