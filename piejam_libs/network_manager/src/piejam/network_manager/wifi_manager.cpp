// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2026  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/network_manager/wifi_manager.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>

namespace piejam::network_manager
{

namespace
{

/// Execute a shell command and return output
std::string
execute_command(char const* cmd)
{
    std::array<char, 256> buffer;
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

/// Execute wpa_cli command
std::string
wpa_cli(std::string const& interface, std::string const& cmd)
{
    std::string full_cmd = "wpa_cli -i " + interface + " " + cmd + " 2>/dev/null";
    return execute_command(full_cmd.c_str());
}

/// Parse security flags from wpa_cli scan results
wifi_security
parse_security_flags(std::string const& flags)
{
    if (flags.find("WPA3") != std::string::npos)
        return wifi_security::wpa3;
    if (flags.find("WPA2") != std::string::npos)
        return wifi_security::wpa2;
    if (flags.find("WPA") != std::string::npos)
        return wifi_security::wpa;
    if (flags.find("WEP") != std::string::npos)
        return wifi_security::wep;
    if (flags.find("ESS") != std::string::npos && flags.find("WPA") == std::string::npos)
        return wifi_security::open;
    return wifi_security::unknown;
}

/// Parse scan results from wpa_cli
std::vector<wifi_network>
parse_scan_results(std::string const& output)
{
    std::vector<wifi_network> networks;
    std::istringstream stream(output);
    std::string line;

    // Skip header line
    std::getline(stream, line);

    // Format: bssid / frequency / signal level / flags / ssid
    while (std::getline(stream, line))
    {
        if (line.empty())
            continue;

        std::istringstream line_stream(line);
        wifi_network net;

        std::string freq_str;
        std::string signal_str;
        std::string flags;

        line_stream >> net.bssid >> freq_str >> signal_str >> flags;

        // Rest of line is SSID (may contain spaces)
        std::getline(line_stream >> std::ws, net.ssid);

        if (net.ssid.empty() || net.bssid.empty())
            continue;

        try
        {
            net.frequency_mhz = std::stoi(freq_str);
            net.signal_strength = std::stoi(signal_str);
        }
        catch (...)
        {
            continue;
        }

        net.signal_percent = signal_dbm_to_percent(net.signal_strength);
        net.band = frequency_to_band(net.frequency_mhz);
        net.security = parse_security_flags(flags);

        networks.push_back(std::move(net));
    }

    // Sort by signal strength (strongest first)
    std::sort(
        networks.begin(),
        networks.end(),
        [](wifi_network const& a, wifi_network const& b) {
            return a.signal_percent > b.signal_percent;
        });

    // Remove duplicates (same SSID, keep strongest signal)
    auto unique_end = std::unique(
        networks.begin(),
        networks.end(),
        [](wifi_network const& a, wifi_network const& b) {
            return a.ssid == b.ssid;
        });
    networks.erase(unique_end, networks.end());

    return networks;
}

/// Get current connection info from wpa_cli status
struct connection_info
{
    std::string ssid;
    std::string bssid;
    std::string ip_address;
    std::string wpa_state;
    int frequency{0};
};

connection_info
get_connection_info(std::string const& interface)
{
    connection_info info;
    std::string output = wpa_cli(interface, "status");
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line))
    {
        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "ssid")
            info.ssid = value;
        else if (key == "bssid")
            info.bssid = value;
        else if (key == "ip_address")
            info.ip_address = value;
        else if (key == "wpa_state")
            info.wpa_state = value;
        else if (key == "freq")
        {
            try
            {
                info.frequency = std::stoi(value);
            }
            catch (...)
            {
            }
        }
    }

    return info;
}

/// Get IP address from interface
std::string
get_ip_address(std::string const& interface)
{
    std::string cmd = "ip -4 addr show " + interface +
                      " | grep -oP '(?<=inet\\s)\\d+(\\.\\d+){3}' 2>/dev/null";
    std::string result = execute_command(cmd.c_str());
    // Remove trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();
    return result;
}

/// Get signal strength of current connection
int
get_current_signal(std::string const& interface)
{
    std::string output = wpa_cli(interface, "signal_poll");
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line))
    {
        if (line.find("RSSI=") == 0)
        {
            try
            {
                int rssi = std::stoi(line.substr(5));
                return signal_dbm_to_percent(rssi);
            }
            catch (...)
            {
            }
        }
    }
    return 0;
}

/// Parse saved networks from wpa_cli list_networks
std::vector<wifi_saved_network>
parse_saved_networks(std::string const& output)
{
    std::vector<wifi_saved_network> networks;
    std::istringstream stream(output);
    std::string line;

    // Skip header line
    std::getline(stream, line);

    // Format: network id / ssid / bssid / flags
    while (std::getline(stream, line))
    {
        if (line.empty())
            continue;

        std::istringstream line_stream(line);
        wifi_saved_network net;

        std::string id;
        std::string bssid;
        std::string flags;

        line_stream >> id >> net.ssid >> bssid >> flags;

        if (net.ssid.empty())
            continue;

        // Parse flags for disabled status
        net.auto_connect = (flags.find("DISABLED") == std::string::npos);

        networks.push_back(std::move(net));
    }

    return networks;
}

/// Add or update network in wpa_supplicant
int
add_network_to_wpa(
    std::string const& interface,
    std::string const& ssid,
    std::string const& password,
    wifi_security security)
{
    // Add new network
    std::string result = wpa_cli(interface, "add_network");
    int network_id = -1;
    try
    {
        network_id = std::stoi(result);
    }
    catch (...)
    {
        return -1;
    }

    std::string id_str = std::to_string(network_id);

    // Set SSID
    wpa_cli(interface, "set_network " + id_str + " ssid '\"" + ssid + "\"'");

    // Set security
    if (security == wifi_security::open || password.empty())
    {
        wpa_cli(interface, "set_network " + id_str + " key_mgmt NONE");
    }
    else
    {
        wpa_cli(interface, "set_network " + id_str + " psk '\"" + password + "\"'");
    }

    // Enable network
    wpa_cli(interface, "enable_network " + id_str);

    return network_id;
}

} // namespace

struct wifi_manager::impl
{
    std::string interface;
    wifi_connection_status connection_status{wifi_connection_status::disconnected};
    std::vector<wifi_network> cached_networks;
    wifi_network current_network;
    bool auto_reconnect_enabled{true};

    scan_complete_callback on_scan_complete;
    connection_changed_callback on_connection_changed;
    error_callback on_error;

    explicit impl(std::string iface)
        : interface(std::move(iface))
    {
        // Check initial connection status
        auto info = get_connection_info(interface);
        if (info.wpa_state == "COMPLETED")
        {
            connection_status = wifi_connection_status::connected;
            current_network.ssid = info.ssid;
            current_network.bssid = info.bssid;
            current_network.frequency_mhz = info.frequency;
            current_network.band = frequency_to_band(info.frequency);
            current_network.is_connected = true;
        }

        spdlog::info(
            "WiFi manager initialized for interface {}, status: {}",
            interface,
            info.wpa_state);
    }

    void update_connection_status()
    {
        auto info = get_connection_info(interface);
        wifi_connection_status new_status;

        if (info.wpa_state == "COMPLETED")
        {
            new_status = wifi_connection_status::connected;
            current_network.ssid = info.ssid;
            current_network.bssid = info.bssid;
            current_network.frequency_mhz = info.frequency;
            current_network.band = frequency_to_band(info.frequency);
            current_network.is_connected = true;
        }
        else if (info.wpa_state == "ASSOCIATING" || info.wpa_state == "ASSOCIATED" ||
                 info.wpa_state == "4WAY_HANDSHAKE" ||
                 info.wpa_state == "GROUP_HANDSHAKE")
        {
            new_status = wifi_connection_status::connecting;
        }
        else
        {
            new_status = wifi_connection_status::disconnected;
            current_network = {};
        }

        if (new_status != connection_status)
        {
            connection_status = new_status;
            if (on_connection_changed)
            {
                on_connection_changed(
                    connection_status,
                    connection_status == wifi_connection_status::connected
                        ? &current_network
                        : nullptr);
            }
        }
    }
};

wifi_manager::wifi_manager(std::string interface)
    : m_impl(std::make_unique<impl>(std::move(interface)))
{
}

wifi_manager::~wifi_manager() = default;

wifi_manager::wifi_manager(wifi_manager&&) noexcept = default;
wifi_manager& wifi_manager::operator=(wifi_manager&&) noexcept = default;

wifi_connection_status
wifi_manager::status() const noexcept
{
    return m_impl->connection_status;
}

std::string
wifi_manager::current_ssid() const
{
    return m_impl->current_network.ssid;
}

std::string
wifi_manager::ip_address() const
{
    return get_ip_address(m_impl->interface);
}

int
wifi_manager::signal_strength() const
{
    if (m_impl->connection_status != wifi_connection_status::connected)
        return 0;
    return get_current_signal(m_impl->interface);
}

bool
wifi_manager::is_interface_available() const
{
    std::string cmd = "ip link show " + m_impl->interface + " 2>/dev/null";
    std::string result = execute_command(cmd.c_str());
    return !result.empty();
}

void
wifi_manager::scan_networks()
{
    spdlog::debug("Starting WiFi scan on {}", m_impl->interface);

    // Trigger scan
    std::string result = wpa_cli(m_impl->interface, "scan");
    if (result.find("OK") == std::string::npos)
    {
        spdlog::warn("Failed to initiate WiFi scan");
        if (m_impl->on_error)
            m_impl->on_error("Failed to initiate WiFi scan");
        return;
    }

    // Wait briefly for scan to complete
    // In production, this should be async with event monitoring
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Get results
    result = wpa_cli(m_impl->interface, "scan_results");
    m_impl->cached_networks = parse_scan_results(result);

    // Mark current network as connected
    for (auto& net : m_impl->cached_networks)
    {
        if (net.ssid == m_impl->current_network.ssid)
        {
            net.is_connected = true;
        }
    }

    spdlog::info("WiFi scan complete, found {} networks", m_impl->cached_networks.size());

    if (m_impl->on_scan_complete)
    {
        m_impl->on_scan_complete(m_impl->cached_networks);
    }
}

std::vector<wifi_network> const&
wifi_manager::available_networks() const
{
    return m_impl->cached_networks;
}

wifi_connection_result
wifi_manager::connect(
    std::string const& ssid,
    std::string const& password,
    bool remember)
{
    wifi_connection_result result;
    result.ssid = ssid;

    spdlog::info("Connecting to WiFi network: {}", ssid);

    // Check if network is already saved
    auto saved = saved_networks();
    int network_id = -1;

    for (size_t i = 0; i < saved.size(); ++i)
    {
        if (saved[i].ssid == ssid)
        {
            network_id = static_cast<int>(i);
            break;
        }
    }

    // Find network in scan results to get security type
    wifi_security security = wifi_security::wpa2; // Default assumption
    for (auto const& net : m_impl->cached_networks)
    {
        if (net.ssid == ssid)
        {
            security = net.security;
            break;
        }
    }

    // Add network if not saved, or if we want to remember with new password
    if (network_id < 0 || remember)
    {
        network_id = add_network_to_wpa(m_impl->interface, ssid, password, security);
        if (network_id < 0)
        {
            result.error_message = "Failed to add network to wpa_supplicant";
            spdlog::error("{}", result.error_message);
            if (m_impl->on_error)
                m_impl->on_error(result.error_message);
            return result;
        }
    }

    // Select and connect to network
    std::string id_str = std::to_string(network_id);
    wpa_cli(m_impl->interface, "select_network " + id_str);

    // Wait for connection (with timeout)
    m_impl->connection_status = wifi_connection_status::connecting;
    if (m_impl->on_connection_changed)
    {
        m_impl->on_connection_changed(m_impl->connection_status, nullptr);
    }

    constexpr int max_attempts = 15;
    for (int i = 0; i < max_attempts; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        m_impl->update_connection_status();

        if (m_impl->connection_status == wifi_connection_status::connected)
        {
            result.success = true;
            result.ip_address = ip_address();

            if (remember)
            {
                wpa_cli(m_impl->interface, "save_config");
            }

            spdlog::info("Connected to {} with IP {}", ssid, result.ip_address);
            return result;
        }
    }

    // Connection failed
    m_impl->connection_status = wifi_connection_status::failed;
    result.error_message = "Connection timeout";
    spdlog::warn("Failed to connect to {}: {}", ssid, result.error_message);

    if (m_impl->on_connection_changed)
    {
        m_impl->on_connection_changed(m_impl->connection_status, nullptr);
    }

    if (!remember)
    {
        // Remove network if we don't want to remember it
        wpa_cli(m_impl->interface, "remove_network " + id_str);
    }

    return result;
}

void
wifi_manager::disconnect()
{
    spdlog::info("Disconnecting from WiFi");
    wpa_cli(m_impl->interface, "disconnect");
    m_impl->update_connection_status();
}

void
wifi_manager::reconnect()
{
    spdlog::info("Reconnecting to WiFi");
    wpa_cli(m_impl->interface, "reconnect");
    m_impl->update_connection_status();
}

std::vector<wifi_saved_network>
wifi_manager::saved_networks() const
{
    std::string output = wpa_cli(m_impl->interface, "list_networks");
    return parse_saved_networks(output);
}

bool
wifi_manager::forget_network(std::string const& ssid)
{
    spdlog::info("Forgetting network: {}", ssid);

    auto networks = saved_networks();
    for (size_t i = 0; i < networks.size(); ++i)
    {
        if (networks[i].ssid == ssid)
        {
            wpa_cli(m_impl->interface, "remove_network " + std::to_string(i));
            wpa_cli(m_impl->interface, "save_config");
            return true;
        }
    }

    return false;
}

void
wifi_manager::set_network_priority(std::string const& ssid, int priority)
{
    auto networks = saved_networks();
    for (size_t i = 0; i < networks.size(); ++i)
    {
        if (networks[i].ssid == ssid)
        {
            wpa_cli(
                m_impl->interface,
                "set_network " + std::to_string(i) + " priority " +
                    std::to_string(priority));
            wpa_cli(m_impl->interface, "save_config");
            return;
        }
    }
}

void
wifi_manager::set_network_auto_connect(std::string const& ssid, bool enabled)
{
    auto networks = saved_networks();
    for (size_t i = 0; i < networks.size(); ++i)
    {
        if (networks[i].ssid == ssid)
        {
            if (enabled)
            {
                wpa_cli(m_impl->interface, "enable_network " + std::to_string(i));
            }
            else
            {
                wpa_cli(m_impl->interface, "disable_network " + std::to_string(i));
            }
            wpa_cli(m_impl->interface, "save_config");
            return;
        }
    }
}

void
wifi_manager::set_auto_reconnect(bool enabled)
{
    m_impl->auto_reconnect_enabled = enabled;
    spdlog::info("Auto-reconnect {}", enabled ? "enabled" : "disabled");
}

bool
wifi_manager::auto_reconnect() const noexcept
{
    return m_impl->auto_reconnect_enabled;
}

void
wifi_manager::set_scan_complete_callback(scan_complete_callback cb)
{
    m_impl->on_scan_complete = std::move(cb);
}

void
wifi_manager::set_connection_changed_callback(connection_changed_callback cb)
{
    m_impl->on_connection_changed = std::move(cb);
}

void
wifi_manager::set_error_callback(error_callback cb)
{
    m_impl->on_error = std::move(cb);
}

} // namespace piejam::network_manager
