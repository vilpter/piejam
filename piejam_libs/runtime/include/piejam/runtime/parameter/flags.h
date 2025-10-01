// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/assert.hpp>

#include <bitset>
#include <type_traits>
#include <utility>

namespace piejam::runtime::parameter
{

template <class E>
concept flags_enum = std::is_scoped_enum_v<E> &&
                     std::is_same_v<std::underlying_type_t<E>, std::size_t>;

struct flags_set
{
    static constexpr std::size_t max_num_flags{64};

    constexpr flags_set() noexcept = default;

    constexpr auto operator==(flags_set const&) const noexcept
            -> bool = default;

    template <flags_enum... E>
    constexpr flags_set(E... f) noexcept
    {
        (set(f), ...);
    }

    template <flags_enum... E>
    constexpr auto set(E... f) noexcept -> flags_set&
    {
        BOOST_ASSERT(((std::to_underlying(f) < max_num_flags) && ...));
        (fs.set(std::to_underlying(f)), ...);
        return *this;
    }

    template <flags_enum E>
    constexpr auto test(E f) const noexcept -> bool
    {
        BOOST_ASSERT(std::to_underlying(f) < max_num_flags);
        return fs.test(std::to_underlying(f));
    }

private:
    std::bitset<max_num_flags> fs{};
};

} // namespace piejam::runtime::parameter
