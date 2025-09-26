// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/Log.h>

#include <piejam/gui/model/StringList.h>

#include <piejam/log/generic_log_sink.h>

#include <spdlog/spdlog.h>

namespace piejam::gui::model
{

struct Log::Impl
{
    StringList messages{};
};

Log::Log(runtime::state_access const& state_access)
    : SubscribableModel{state_access}
    , m_impl{make_pimpl<Impl>()}
{
    spdlog::default_logger()->sinks().push_back(
            std::make_shared<log::generic_log_sink_mt>(
                    [this](spdlog::details::log_msg const& msg) {
                        auto qtMsg = QString::fromStdString(
                                std::format(
                                        "[{}] {}",
                                        spdlog::level::to_string_view(
                                                msg.level),
                                        msg.payload));
                        m_impl->messages.add(m_impl->messages.size(), qtMsg);
                    },
                    []() {}));
}

auto
Log::logMessages() const noexcept -> logMessages_property_t
{
    return &m_impl->messages;
}

void
Log::onSubscribe()
{
}

} // namespace piejam::gui::model
