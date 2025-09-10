// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <type_traits>
#include <utility>

namespace piejam
{

template <class E>
concept bool_enum = std::is_scoped_enum_v<E> &&
                    std::is_same_v<std::underlying_type_t<E>, bool>;

template <bool_enum E, class T>
[[nodiscard]]
constexpr auto
bool_enum_to(E e, T&& false_value, T&& true_value) noexcept -> decltype(auto)
{
    return static_cast<bool>(e) ? std::forward<T>(true_value)
                                : std::forward<T>(false_value);
}

template <bool_enum ToE, bool_enum FromE>
[[nodiscard]]
constexpr auto
bool_enum_to(FromE e) noexcept -> decltype(auto)
{
    return static_cast<ToE>(static_cast<bool>(e));
}

} // namespace piejam
