// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <concepts>
#include <iterator>

namespace piejam::algorithm
{

template <
        std::input_iterator InputIt,
        typename OutputIt,
        typename T,
        std::invocable<std::iter_reference_t<InputIt>> UnaryOp,
        std::invocable<
                T,
                std::invoke_result_t<UnaryOp, std::iter_reference_t<InputIt>>>
                BinaryOp>
    requires requires(
            OutputIt it,
            std::invoke_result_t<UnaryOp, std::iter_reference_t<InputIt>> val) {
        *it = val; // output iterator must be assignable from unary result
    }
auto
transform_accumulate(
        InputIt first,
        InputIt last,
        OutputIt d_first,
        T init,
        UnaryOp unary_op,
        BinaryOp binary_op) -> T
{
    for (; first != last; ++first, ++d_first)
    {
        auto const tmp = unary_op(*first); // transform
        *d_first = tmp;                    // write to output
        init = binary_op(init, tmp);       // accumulate
    }
    return init;
}

} // namespace piejam::algorithm
