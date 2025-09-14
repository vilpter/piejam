// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include "get_sound_cards.h"

#include <piejam/box.h>
#include <piejam/io_pair.h>
#include <piejam/system/device.h>

#include <spdlog/spdlog.h>

#include <sound/asound.h>
#include <sys/ioctl.h>

#include <boost/assert.hpp>

#include <filesystem>
#include <format>

namespace piejam::audio::alsa
{

namespace
{

struct sound_card_info
{
    std::filesystem::path control_path;
    snd_ctl_card_info info;
};

std::filesystem::path const s_sound_cards_dir("/dev/snd");

auto
scan_for_sound_cards() -> std::vector<sound_card_info>
{
    std::vector<sound_card_info> cards;

    std::error_code ec;
    if (std::filesystem::exists(s_sound_cards_dir, ec))
    {
        for (auto const& entry :
             std::filesystem::directory_iterator(s_sound_cards_dir, ec))
        {
            unsigned card{};
            if (1 == std::sscanf(
                             entry.path().filename().c_str(),
                             "controlC%u",
                             &card))
            {
                system::device fd(entry.path());
                snd_ctl_card_info card_info{};

                if (auto err = fd.ioctl(SNDRV_CTL_IOCTL_CARD_INFO, card_info))
                {
                    auto const message = err.message();
                    spdlog::error("scan_for_sound_cards: {}", message);
                }
                else
                {
                    cards.emplace_back(
                            sound_card_info{
                                    .control_path = entry.path(),
                                    .info = card_info});
                }
            }
        }
    }

    return cards;
}

auto
get_sound_card_descriptors(sound_card_info const& sc)
        -> std::vector<sound_card_descriptor>
{
    std::vector<sound_card_descriptor> result;

    system::device fd(sc.control_path);

    int device{-1};
    do
    {
        if (auto err = fd.ioctl(SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE, device))
        {
            auto const message = err.message();
            spdlog::error("get_sound_card_pcm_infos: {}", message);
            break;
        }

        if (device == -1)
        {
            break;
        }

        sound_card_descriptor desc{};

        auto get_stream = [&](int stream_type) {
            snd_pcm_info info{};
            info.card = sc.info.card;
            info.device = static_cast<unsigned>(device);
            info.subdevice = 0;
            info.stream = stream_type;

            if (auto err = fd.ioctl(SNDRV_CTL_IOCTL_PCM_INFO, info))
            {
                static auto const error_to_ignore = std::make_error_code(
                        std::errc::no_such_file_or_directory);
                if (err != error_to_ignore)
                {
                    auto const message = err.message();
                    spdlog::error("get_sound_card_pcm_infos: {}", message);
                }

                return std::filesystem::path{};
            }

            desc.name = std::format(
                    "{} - {}",
                    reinterpret_cast<char const*>(sc.info.name),
                    reinterpret_cast<char const*>(info.name));
            return std::filesystem::path{std::format(
                    "/dev/snd/pcmC{}D{}{}",
                    sc.info.card,
                    info.device,
                    stream_type == SNDRV_PCM_STREAM_CAPTURE ? 'c' : 'p')};
        };

        desc.streams.in.device_path = get_stream(SNDRV_PCM_STREAM_CAPTURE);
        desc.streams.out.device_path = get_stream(SNDRV_PCM_STREAM_PLAYBACK);
        result.emplace_back(std::move(desc));

    } while (device != -1);

    return result;
}

} // namespace

auto
get_sound_cards() -> sound_cards
{
    std::vector<sound_card_descriptor> result;

    for (sound_card_info const& sc_info : scan_for_sound_cards())
    {
        result.append_range(get_sound_card_descriptors(sc_info));
    }

    return result;
}

} // namespace piejam::audio::alsa
