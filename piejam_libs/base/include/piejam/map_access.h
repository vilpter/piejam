// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/assert.hpp>
#include <boost/hof/pipable.hpp>

#include <utility>

namespace piejam
{

struct map_at_f
{
    template <class Map, class Key>
        requires requires(Map&& m, Key&& k) { m.find(std::forward<Key>(k)); }
    constexpr auto operator()(Map&& m, Key&& key) const -> decltype(auto)
    {
        auto it = m.find(std::forward<Key>(key));
        BOOST_ASSERT(it != m.end());
        return it->second;
    }
};

inline constexpr auto map_at = boost::hof::pipable(map_at_f{});

} // namespace piejam
