// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/audio/sound_card_manager.h>

#include "alsa/get_set_hw_params.h"
#include "alsa/get_sound_cards.h"
#include "alsa/pcm_io.h"

#include <piejam/audio/sound_card_descriptor.h>
#include <piejam/audio/sound_card_stream_hw_params.h>

#include <piejam/box.h>
#include <piejam/io_pair.h>

namespace piejam::audio
{

namespace
{

class alsa_sound_card_manager final : public sound_card_manager
{
public:
    auto get_sound_cards() -> sound_cards override
    {
        return alsa::get_sound_cards();
    }

    auto hw_params(
            sound_card_stream_descriptor const& d,
            sample_rate const* const sample_rate,
            period_size const* const period_size)
            -> sound_card_stream_hw_params override
    {
        return alsa::get_hw_params(d, sample_rate, period_size);
    }

    auto make_io_process(
            sound_card_stream_descriptor const& in,
            sound_card_stream_descriptor const& out,
            io_process_config const& config)
            -> std::unique_ptr<io_process> override
    {
        return std::make_unique<alsa::pcm_io>(in, out, config);
    }
};

} // namespace

auto
get_default_sound_card_manager() -> sound_card_manager&
{
    static auto s_instance = alsa_sound_card_manager{};
    return s_instance;
}

} // namespace piejam::audio
