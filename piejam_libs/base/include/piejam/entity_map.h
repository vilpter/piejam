// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/box.h>
#include <piejam/entity_id.h>
#include <piejam/fwd.h>

#include <boost/assert.hpp>
#include <boost/container/flat_map.hpp>

namespace piejam
{

template <class Entity, class Id>
class entity_map
{
    static constexpr bool foreign_id = !std::is_same_v<Id, entity_id<Entity>>;

public:
    using id_t = Id;
    using map_t = boost::container::flat_map<id_t, Entity>;
    using value_type = typename map_t::value_type;

    [[nodiscard]]
    auto empty() const noexcept
    {
        return m_map.empty();
    }

    [[nodiscard]]
    auto size() const noexcept
    {
        return m_map.size();
    }

    [[nodiscard]]
    auto begin() const noexcept
    {
        return m_map.begin();
    }

    // TODO: remove
    [[nodiscard]]
    auto begin() noexcept
    {
        return m_map.begin();
    }

    [[nodiscard]]
    auto end() const noexcept
    {
        return m_map.end();
    }

    // TODO: remove
    [[nodiscard]]
    auto end() noexcept
    {
        return m_map.end();
    }

    [[nodiscard]]
    auto contains(id_t id) const noexcept
    {
        return m_map.contains(id);
    }

    [[nodiscard]]
    auto find(id_t id) const noexcept
    {
        return m_map.find(id);
    }

    // TODO: remove
    [[nodiscard]]
    auto find(id_t id) noexcept
    {
        return m_map.find(id);
    }

    template <class... Args>
    [[nodiscard]]
    auto emplace(Args&&... args) -> id_t
        requires(!foreign_id)
    {
        auto id = id_t::generate();
        m_map.emplace_hint(
                m_map.end(),
                std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(std::forward<Args>(args)...));
        return id;
    }

    template <class... Args>
    auto emplace(id_t id, Args&&... args) -> bool
        requires(foreign_id)
    {
        return m_map
                .emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(id),
                        std::forward_as_tuple(std::forward<Args>(args)...))
                .second;
    }

    auto erase(id_t id) -> typename map_t::size_type
    {
        return m_map.erase(id);
    }

    // TODO remove
    template <std::ranges::range RangeOfIds>
    void erase(RangeOfIds&& ids)
    {
        m_map.erase(std::ranges::begin(ids), std::ranges::end(ids));
    }

    auto operator==(entity_map const&) const noexcept -> bool = default;

private:
    map_t m_map;
};

} // namespace piejam
