// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/box.h>
#include <piejam/lean_map_facade.h>

namespace piejam
{

template <class Map>
class boxed_map
{
    using map_t = lean_map_facade<Map>;

public:
    using key_type = typename map_t::key_type;
    using mapped_type = typename map_t::mapped_type;
    using size_type = typename map_t::size_type;
    using const_iterator = typename map_t::const_iterator;
    using iterator = typename map_t::iterator;

    [[nodiscard]]
    auto empty() const noexcept -> bool
    {
        return m_map->empty();
    }

    [[nodiscard]]
    auto size() const noexcept -> size_type
    {
        return m_map->size();
    }

    [[nodiscard]]
    auto begin() const noexcept -> const_iterator
    {
        return m_map->begin();
    }

    [[nodiscard]]
    auto end() const noexcept -> const_iterator
    {
        return m_map->end();
    }

    template <class Key>
    [[nodiscard]]
    auto contains(Key&& key) const noexcept -> bool
    {
        return m_map->contains(std::forward<Key>(key));
    }

    template <class Key>
    [[nodiscard]]
    auto find(Key&& key) const noexcept -> mapped_type const*
    {
        return m_map->find(std::forward<Key>(key));
    }

    template <class Key>
    [[nodiscard]]
    auto at(Key&& key) const noexcept -> mapped_type const&
    {
        return m_map->at(std::forward<Key>(key));
    }

    class locked
    {
    public:
        explicit locked(boxed_map& m)
            : m_locked{m.m_map.lock()}
        {
        }

        [[nodiscard]]
        auto begin() noexcept -> iterator
        {
            return m_locked->begin();
        }

        [[nodiscard]]
        auto end() noexcept -> iterator
        {
            return m_locked->end();
        }

        template <class Key>
        [[nodiscard]]
        auto find(Key&& key) -> mapped_type*
        {
            return m_locked->find(std::forward<Key>(key));
        }

        template <class Key>
        [[nodiscard]]
        auto at(Key&& key) -> mapped_type&
        {
            return m_locked->at(std::forward<Key>(key));
        }

        template <class... Args>
        auto emplace(Args&&... args)
        {
            return m_locked->emplace(std::forward<Args>(args)...);
        }

        template <class Key>
        auto erase(Key&& key)
        {
            return m_locked->erase(std::forward<Key>(key));
        }

    private:
        box<map_t>::write_lock m_locked;
    };

    auto lock() -> locked
    {
        return locked{*this};
    }

    template <class Key, class Value>
    auto assign(Key&& key, Value&& value)
    {
        lock().at(std::forward<Key>(key)) = std::forward<Value>(value);
    }

    template <class... Args>
    auto emplace(Args&&... args)
    {
        return lock().emplace(std::forward<Args>(args)...);
    }

    template <class Key>
    auto erase(Key&& id)
    {
        return lock().erase(id);
    }

    auto operator==(boxed_map const&) const noexcept -> bool = default;

private:
    box<map_t> m_map;
};

} // namespace piejam
