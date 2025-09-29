// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/box.h>

namespace piejam
{

template <class Map>
class boxed_map
{
public:
    [[nodiscard]]
    auto empty() const noexcept
    {
        return m_map->empty();
    }

    [[nodiscard]]
    auto size() const noexcept
    {
        return m_map->size();
    }

    [[nodiscard]]
    auto begin() const noexcept
    {
        return m_map->begin();
    }

    [[nodiscard]]
    auto end() const noexcept
    {
        return m_map->end();
    }

    template <class Key>
    [[nodiscard]]
    auto contains(Key&& key) const noexcept
    {
        return m_map->contains(std::forward<Key>(key));
    }

    template <class Key>
    [[nodiscard]]
    auto find(Key&& key) const noexcept
    {
        auto it = m_map->find(std::forward<Key>(key));
        return it != m_map->end() ? std::addressof(it->second) : nullptr;
    }

    template <class Key>
    [[nodiscard]]
    auto operator[](Key&& key) const noexcept -> decltype(auto)
    {
        auto it = m_map->find(std::forward<Key>(key));
        BOOST_ASSERT(it != m_map->end());
        auto const& ref = it->second;
        return ref;
    }

    class locked
    {
    public:
        explicit locked(boxed_map& m)
            : m_{m.m_map.lock()}
        {
        }

        [[nodiscard]]
        auto begin() noexcept
        {
            return m_->begin();
        }

        [[nodiscard]]
        auto end() noexcept
        {
            return m_->end();
        }

        template <class Key>
        [[nodiscard]]
        auto operator[](Key&& key) -> decltype(auto)
        {
            auto it = m_->find(std::forward<Key>(key));
            BOOST_ASSERT(it != m_->end());
            auto& ref = it->second;
            return ref;
        }

        template <class... Args>
        auto emplace(Args&&... args)
        {
            return m_->emplace(std::forward<Args>(args)...);
        }

        template <class Key>
        auto erase(Key&& key)
        {
            return m_->erase(std::forward<Key>(key));
        }

        // TODO remove
        template <std::ranges::range RangeOfKeys>
        void erase(RangeOfKeys&& keys)
        {
            for (auto&& key : keys)
            {
                m_->erase(key);
            }
        }

    private:
        box<Map>::write_lock m_;
    };

    auto lock() -> locked
    {
        return locked{*this};
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

    // TODO remove
    template <std::ranges::range RangeOfIds>
    void erase(RangeOfIds&& ids)
    {
        lock().erase(std::forward<RangeOfIds>(ids));
    }

    auto operator==(boxed_map const&) const noexcept -> bool = default;

private:
    box<Map> m_map;
};

} // namespace piejam
