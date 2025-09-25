// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/MixerChannel.h>

#include <piejam/runtime/selectors.h>

namespace piejam::gui::model
{

namespace
{

constexpr auto
toChannelType(runtime::mixer::channel_type t) -> ChannelType
{
    switch (t)
    {
        case runtime::mixer::channel_type::mono:
            return ChannelType::Mono;

        case runtime::mixer::channel_type::stereo:
            return ChannelType::Stereo;

        case runtime::mixer::channel_type::aux:
            return ChannelType::Aux;
    }

    return ChannelType::Mono;
}

} // namespace

MixerChannel::MixerChannel(
        runtime::state_access const& state_access,
        runtime::mixer::channel_id const id)
    : SubscribableModel{state_access}
    , m_color{static_cast<MaterialColor>(observe_once(
              runtime::selectors::make_mixer_channel_color_selector(id)))}
    , m_channel_id{id}
    , m_channelType{toChannelType(observe_once(
              runtime::selectors::make_mixer_channel_type_selector(id)))}
    , m_busType{bool_enum_to<BusType>(to_bus_type(observe_once(
              runtime::selectors::make_mixer_channel_type_selector(id))))}
{
}

MixerChannel::~MixerChannel() = default;

auto
MixerChannel::channelType() const noexcept -> ChannelType
{
    return m_channelType;
}

auto
MixerChannel::busType() const noexcept -> BusType
{
    return m_busType;
}

void
MixerChannel::onSubscribe()
{
    observe(runtime::selectors::make_mixer_channel_name_string_selector(
                    m_channel_id),
            [this](boxed_string const& name) {
                setName(QString::fromStdString(*name));
            });

    observe(runtime::selectors::make_mixer_channel_color_selector(m_channel_id),
            [this](runtime::material_color color) {
                setColor(static_cast<MaterialColor>(color));
            });
}

} // namespace piejam::gui::model
