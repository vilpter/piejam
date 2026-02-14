// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2026  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/network_manager/fwd.h>
#include <piejam/network_manager/network_controller.h>
#include <piejam/redux/middleware_functors.h>

#include <functional>
#include <memory>

namespace piejam::network_manager
{

/// Middleware that handles network enable/disable based on recording state.
///
/// This middleware intercepts recording start/stop actions and manages
/// network state accordingly when auto-disable is enabled.
///
/// It must be placed in the middleware chain BEFORE the recorder_middleware
/// so it can disable network before recording actually starts.
///
/// Usage:
/// @code
/// // In main.cpp, create the middleware with action type checkers
/// auto network_ctrl = std::make_shared<network_controller>();
/// store.apply_middleware(
///     middleware_factory::make<network_middleware<State, Action>>(
///         network_ctrl,
///         [](Action const& a) { return dynamic_cast<start_recording const*>(&a) != nullptr; },
///         [](Action const& a) { return dynamic_cast<stop_recording const*>(&a) != nullptr; }
///     ));
/// @endcode
template <class State, class Action>
class network_middleware final
{
public:
    using action_checker = std::function<bool(Action const&)>;

    /// Construct middleware with network controller and action type checkers.
    /// @param ctrl The network controller to manage
    /// @param is_start_recording Function to check if action is start_recording
    /// @param is_stop_recording Function to check if action is stop_recording
    network_middleware(
        std::shared_ptr<network_controller> ctrl,
        action_checker is_start_recording,
        action_checker is_stop_recording)
        : m_controller(std::move(ctrl))
        , m_is_start_recording(std::move(is_start_recording))
        , m_is_stop_recording(std::move(is_stop_recording))
    {
    }

    void operator()(
        redux::middleware_functors<State, Action> const& mf,
        Action const& action)
    {
        // Check if this is a recording-related action
        if (m_is_start_recording && m_is_start_recording(action))
        {
            // Store previous state and potentially disable network
            m_was_enabled_before_recording = m_controller->on_recording_start();
        }
        else if (m_is_stop_recording && m_is_stop_recording(action))
        {
            // Restore network state after recording
            m_controller->on_recording_stop(m_was_enabled_before_recording);
        }

        // Always pass the action through
        mf.next(action);
    }

    /// Access the network controller for configuration
    [[nodiscard]] network_controller& controller() noexcept
    {
        return *m_controller;
    }

    [[nodiscard]] network_controller const& controller() const noexcept
    {
        return *m_controller;
    }

private:
    std::shared_ptr<network_controller> m_controller;
    action_checker m_is_start_recording;
    action_checker m_is_stop_recording;
    bool m_was_enabled_before_recording{true};
};

} // namespace piejam::network_manager
