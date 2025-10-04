// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/assert.hpp>

#include <utility>

namespace piejam
{

template <class Map>
class lean_map_facade
{
public:
    using key_type = typename Map::key_type;
    using mapped_type = typename Map::mapped_type;
    using size_type = typename Map::size_type;
    using const_iterator = typename Map::const_iterator;
    using iterator = typename Map::iterator;

    constexpr lean_map_facade() noexcept = default;

    constexpr lean_map_facade(
            std::initializer_list<std::pair<key_type, mapped_type>> init)
        : m_map{std::move(init)}
    {
    }

    [[nodiscard]]
    constexpr auto empty() const noexcept -> bool
    {
        return m_map.empty();
    }

    [[nodiscard]]
    constexpr auto size() const noexcept -> size_type
    {
        return m_map.size();
    }

    [[nodiscard]]
    constexpr auto begin() const noexcept -> const_iterator
    {
        return m_map.begin();
    }

    [[nodiscard]]
    constexpr auto begin() noexcept -> iterator
    {
        return m_map.begin();
    }

    [[nodiscard]]
    constexpr auto end() const noexcept -> const_iterator
    {
        return m_map.end();
    }

    [[nodiscard]]
    constexpr auto end() noexcept -> iterator
    {
        return m_map.end();
    }

    template <class Key>
    [[nodiscard]]
    constexpr auto contains(Key&& key) const noexcept -> bool
    {
        return m_map.contains(std::forward<Key>(key));
    }

    template <class Key>
    [[nodiscard]]
    constexpr auto find(Key&& key) const noexcept -> mapped_type const*
    {
        auto it = m_map.find(std::forward<Key>(key));
        return it != m_map.end() ? &it->second : nullptr;
    }

    template <class Key>
    [[nodiscard]]
    constexpr auto find(Key&& key) noexcept -> mapped_type*
    {
        auto it = m_map.find(std::forward<Key>(key));
        return it != m_map.end() ? &it->second : nullptr;
    }

    template <class Key>
    [[nodiscard]]
    constexpr auto at(Key&& key) const noexcept -> mapped_type const&
    {
        auto it = m_map.find(std::forward<Key>(key));
        BOOST_ASSERT(it != m_map.end());
        return it->second;
    }

    template <class Key>
    [[nodiscard]]
    constexpr auto at(Key&& key) noexcept -> mapped_type&
    {
        auto it = m_map.find(std::forward<Key>(key));
        BOOST_ASSERT(it != m_map.end());
        return it->second;
    }

    template <class... Args>
    constexpr auto emplace(Args&&... args)
    {
        return m_map.emplace(std::forward<Args>(args)...);
    }

    template <class Key>
    constexpr void erase(Key&& key)
    {
        m_map.erase(std::forward<Key>(key));
    }

    constexpr auto operator==(lean_map_facade const&) const noexcept
            -> bool = default;

private:
    Map m_map;
};

} // namespace piejam
