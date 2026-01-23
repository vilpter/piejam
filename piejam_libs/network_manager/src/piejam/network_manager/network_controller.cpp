// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/network_manager/network_controller.h>

#include <spdlog/spdlog.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

namespace piejam::network_manager
{

namespace
{

/// Execute a shell command and return success status
bool
execute_command(char const* cmd)
{
    int result = std::system(cmd);
    return result == 0;
}

/// Execute a shell command and capture output
std::string
execute_command_with_output(char const* cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    auto pipe_deleter = [](FILE* f) { if (f) pclose(f); };
    std::unique_ptr<FILE, decltype(pipe_deleter)> pipe(popen(cmd, "r"), pipe_deleter);
    if (!pipe)
    {
        return result;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) !=
           nullptr)
    {
        result += buffer.data();
    }

    return result;
}

/// Check if WiFi interface is up
bool
is_wifi_interface_up()
{
    // Check if wlan0 is up using ip command
    std::string output =
        execute_command_with_output("ip link show wlan0 2>/dev/null | grep 'state UP'");
    return !output.empty();
}

/// Enable WiFi interface
bool
enable_wifi_interface()
{
    spdlog::info("Enabling WiFi interface");

    // Bring up the interface
    if (!execute_command("sudo ip link set wlan0 up 2>/dev/null"))
    {
        spdlog::warn("Failed to bring up wlan0 interface");
        return false;
    }

    // Restart wpa_supplicant to reconnect
    if (!execute_command("sudo systemctl restart wpa_supplicant 2>/dev/null"))
    {
        spdlog::warn("Failed to restart wpa_supplicant");
        // Not fatal - interface is up
    }

    return true;
}

/// Disable WiFi interface
bool
disable_wifi_interface()
{
    spdlog::info("Disabling WiFi interface");

    // Bring down the interface
    if (!execute_command("sudo ip link set wlan0 down 2>/dev/null"))
    {
        spdlog::warn("Failed to bring down wlan0 interface");
        return false;
    }

    return true;
}

/// Unmount all NFS mounts
void
unmount_all_nfs()
{
    spdlog::info("Unmounting NFS shares");
    execute_command("sudo umount -a -t nfs,nfs4 2>/dev/null");
}

} // namespace

struct network_controller::impl
{
    bool enabled{true};
    bool auto_disable_on_record{false};
    state_change_callback on_state_change;

    impl()
    {
        // Initialize state from actual system state
        enabled = is_wifi_interface_up();
        spdlog::info(
            "Network controller initialized, WiFi {}",
            enabled ? "enabled" : "disabled");
    }
};

network_controller::network_controller()
    : m_impl(std::make_unique<impl>())
{
}

network_controller::~network_controller() = default;

network_controller::network_controller(network_controller&&) noexcept = default;
network_controller&
network_controller::operator=(network_controller&&) noexcept = default;

bool
network_controller::is_enabled() const noexcept
{
    return m_impl->enabled;
}

bool
network_controller::auto_disable_on_record() const noexcept
{
    return m_impl->auto_disable_on_record;
}

bool
network_controller::enable()
{
    if (m_impl->enabled)
    {
        spdlog::debug("Network already enabled");
        return true;
    }

    if (!enable_wifi_interface())
    {
        spdlog::error("Failed to enable network");
        return false;
    }

    m_impl->enabled = true;
    spdlog::info("Network enabled");

    if (m_impl->on_state_change)
    {
        m_impl->on_state_change(true);
    }

    return true;
}

bool
network_controller::disable()
{
    if (!m_impl->enabled)
    {
        spdlog::debug("Network already disabled");
        return true;
    }

    // First unmount NFS to prevent hangs
    unmount_all_nfs();

    if (!disable_wifi_interface())
    {
        spdlog::error("Failed to disable network");
        return false;
    }

    m_impl->enabled = false;
    spdlog::info("Network disabled");

    if (m_impl->on_state_change)
    {
        m_impl->on_state_change(false);
    }

    return true;
}

void
network_controller::set_auto_disable_on_record(bool enabled)
{
    m_impl->auto_disable_on_record = enabled;
    spdlog::info(
        "Auto-disable network on recording: {}",
        enabled ? "enabled" : "disabled");
}

bool
network_controller::on_recording_start()
{
    bool was_enabled = m_impl->enabled;

    if (m_impl->auto_disable_on_record && m_impl->enabled)
    {
        spdlog::info("Auto-disabling network for recording");
        disable();
    }

    return was_enabled;
}

void
network_controller::on_recording_stop(bool restore_to_enabled)
{
    if (m_impl->auto_disable_on_record && restore_to_enabled && !m_impl->enabled)
    {
        spdlog::info("Restoring network after recording");
        enable();
    }
}

void
network_controller::set_state_change_callback(state_change_callback cb)
{
    m_impl->on_state_change = std::move(cb);
}

} // namespace piejam::network_manager
