// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/runtime/mixer.h>

#include <piejam/functional/get.h>
#include <piejam/io_pair.h>

#include <boost/hof/compose.hpp>
#include <boost/hof/unpack.hpp>

#include <algorithm>
#include <ranges>

namespace piejam::runtime::mixer
{

namespace
{

struct channel_io_t
{
    io_address_t in;
    io_address_t out;
    std::vector<channel_id> aux_sends;

    template <io_socket S>
    constexpr auto get() -> io_address_t&
    {
        switch (S)
        {
            case io_socket::in:
                return in;

            case io_socket::out:
                return out;
        }
    }

    template <io_socket S>
    constexpr auto get() const -> io_address_t const&
    {
        switch (S)
        {
            case io_socket::in:
                return in;

            case io_socket::out:
                return out;
        }
    }
};

using channels_io_t = boost::container::flat_map<channel_id, channel_io_t>;

auto
extract_channels_io(channels_t const& channels) -> channels_io_t
{
    namespace bhof = boost::hof;

    auto transformed = std::views::transform(
            channels,
            bhof::unpack([](auto const id, auto const& channel) {
                auto active_aux_sends =
                        *channel.aux_sends |
                        std::views::filter(
                                bhof::compose(
                                        &mixer::aux_send::enabled,
                                        get_by_index<1>)) |
                        std::views::keys | std::ranges::to<std::vector>();

                return std::pair(
                        id,
                        channel_io_t{
                                .in = channel.in,
                                .out = channel.out,
                                .aux_sends = std::move(active_aux_sends)});
            }));

    return channels_io_t(
            boost::container::ordered_unique_range,
            transformed.begin(),
            transformed.end());
}

struct io_graph_node
{
    std::vector<channel_id> children;
    bool finished{};
    bool visited{};
};

using io_graph = boost::container::flat_map<channel_id, io_graph_node>;

auto
make_channels_io_graph(channels_io_t const& channels_io) -> io_graph
{
    io_graph result;
    result.reserve(channels_io.size());

    for (auto const& [id, ch_io] : channels_io)
    {
        result[id];

        if (channel_id const* const in_channel_id =
                    std::get_if<channel_id>(&ch_io.in))
        {
            result[*in_channel_id].children.push_back(id);
        }

        auto add_out_child = [&](channel_id const& out_channel_id) {
            if (std::holds_alternative<mixer::mix_input>(
                        channels_io.at(out_channel_id).in))
            {
                result[id].children.push_back(out_channel_id);
            }
        };

        if (auto const* const out_channel_id =
                    std::get_if<channel_id>(&ch_io.out))
        {
            add_out_child(*out_channel_id);
        }

        for (auto aux : ch_io.aux_sends)
        {
            add_out_child(aux);
        }
    }

    return result;
}

bool
has_cycle(io_graph& g, channel_id const id)
{
    BOOST_ASSERT(g.contains(id));

    auto& node = g[id];

    if (node.finished)
    {
        return false;
    }

    if (node.visited)
    {
        return true;
    }

    node.visited = true;

    for (auto child : node.children)
    {
        if (has_cycle(g, child))
        {
            return true;
        }
    }

    node.finished = true;

    return false;
}

bool
has_cycle(io_graph g)
{
    return std::ranges::any_of(g | std::views::keys, [&g](auto id) {
        return has_cycle(g, id);
    });
}

template <io_socket D>
auto
valid_io_channels(channels_t const& channels, channel_id const ch_id)
        -> std::vector<mixer::channel_id>
{
    auto channels_io = extract_channels_io(channels);

    std::vector<mixer::channel_id> valid_ids;
    for (auto const& [mixer_channel_id, mixer_channel] : channels)
    {
        if (mixer_channel_id == ch_id)
        {
            // mono mixer channels can't have input channels
            if (D == io_socket::in && mixer_channel.type == channel_type::mono)
            {
                return {};
            }

            // otherwise, we can't be our own input
            continue;
        }

        if (D == io_socket::out && mixer_channel.type == channel_type::mono)
        {
            // mono mixer channels can't be targets
            continue;
        }

        auto prev_id =
                std::exchange(channels_io[ch_id].get<D>(), mixer_channel_id);

        if (!has_cycle(make_channels_io_graph(channels_io)))
        {
            valid_ids.push_back(mixer_channel_id);
        }

        channels_io[ch_id].get<D>() = prev_id;
    }

    return valid_ids;
}

} // namespace

auto
is_mix_input_valid(channels_t const& channels, channel_id const ch_id) -> bool
{
    BOOST_ASSERT(channels[ch_id].type != mixer::channel_type::mono);
    auto channels_io = extract_channels_io(channels);
    channels_io[ch_id].in = mixer::mix_input{};
    return !has_cycle(make_channels_io_graph(channels_io));
}

auto
can_toggle_aux(
        channels_t const& channels,
        channel_id const ch_id,
        channel_id const aux_id) -> bool
{
    mixer::channel const* const channel = channels.find(ch_id);
    if (!channel)
    {
        return false;
    }

    auto it_aux_send = channel->aux_sends->find(aux_id);
    if (it_aux_send == channel->aux_sends->end())
    {
        return false;
    }

    if (it_aux_send->second.enabled)
    {
        return true; // we can always disable an enabled aux
    }

    auto channels_io = extract_channels_io(channels);

    channels_io[ch_id].aux_sends.emplace_back(aux_id);
    return !has_cycle(make_channels_io_graph(channels_io));
}

auto
valid_channels(
        io_socket const s,
        channels_t const& channels,
        channel_id const ch_id) -> std::vector<channel_id>
{
    switch (s)
    {
        case io_socket::in:
            return valid_io_channels<io_socket::in>(channels, ch_id);

        case io_socket::out:
            return valid_io_channels<io_socket::out>(channels, ch_id);
    }

    return {};
}

} // namespace piejam::runtime::mixer
