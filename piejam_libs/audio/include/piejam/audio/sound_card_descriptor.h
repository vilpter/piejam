// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/io_pair.h>

#include <filesystem>
#include <string>
#include <vector>

namespace piejam::audio
{

using sound_card_stream_descriptor = std::filesystem::path;

struct sound_card_descriptor
{
    std::string name;
    io_pair<sound_card_stream_descriptor> streams;

    auto operator==(sound_card_descriptor const& other) const noexcept
            -> bool = default;
};

using sound_cards = std::vector<sound_card_descriptor>;

} // namespace piejam::audio
