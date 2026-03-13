// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2026  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/network_manager/wifi_manager.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>

#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

namespace piejam::network_manager
{

namespace
{

/// Raw diagnostic log — bypasses spdlog entirely to detect
/// whether spdlog itself is blocked. Uses POSIX write() which
/// is async-signal-safe and never deadlocks.
void
diag_log(char const* msg)
{
    int fd = ::open(
        "/piejam/diag.log",
        O_WRONLY | O_CREAT | O_APPEND,
        0644);
    if (fd >= 0)
    {
        ::dprintf(fd, "%s\n", msg);
        ::close(fd);
    }
}

void
diag_log_cmd(char const* prefix, char const* cmd)
{
    int fd = ::open(
        "/piejam/diag.log",
        O_WRONLY | O_CREAT | O_APPEND,
        0644);
    if (fd >= 0)
    {
        ::dprintf(fd, "%s: %s\n", prefix, cmd);
        ::close(fd);
    }
}

constexpr int execute_command_timeout_sec = 15;

std::string
execute_command(char const* cmd)
{
    diag_log_cmd("enter execute_command", cmd);

    int pipe_fds[2];
    if (::pipe(pipe_fds) != 0)
    {
        diag_log("pipe() failed");
        return {};
    }

    diag_log_cmd("before fork", cmd);
    pid_t pid = ::fork();
    diag_log_cmd("after fork", cmd);

    if (pid < 0)
    {
        diag_log("fork() failed");
        spdlog::error("fork failed for: {} (errno={})", cmd, errno);
        ::close(pipe_fds[0]);
        ::close(pipe_fds[1]);
        return {};
    }

    if (pid == 0)
    {
        // Child process — only async-signal-safe calls between fork and exec
        ::dup2(pipe_fds[1], STDOUT_FILENO);
        int devnull = ::open("/dev/null", O_WRONLY);
        if (devnull >= 0)
        {
            ::dup2(devnull, STDERR_FILENO);
            ::close(devnull);
        }
        ::close(pipe_fds[0]);
        ::close(pipe_fds[1]);

        ::execl("/bin/sh", "/bin/sh", "-c", cmd, nullptr);
        ::_exit(127);
    }

    // Parent — returns here immediately
    ::close(pipe_fds[1]);

    std::string result;
    diag_log_cmd("parent continues, entering poll loop", cmd);
    spdlog::debug("forked pid {} for: {}", pid, cmd);

    struct pollfd pfd;
    pfd.fd = pipe_fds[0];
    pfd.events = POLLIN;

    std::array<char, 256> buffer;
    bool timed_out = false;

    for (;;)
    {
        int poll_ret = ::poll(
            &pfd,
            1,
            execute_command_timeout_sec * 1000);

        if (poll_ret < 0 && errno == EINTR)
            continue;

        if (poll_ret == 0)
        {
            diag_log_cmd("poll TIMEOUT", cmd);
            spdlog::warn(
                "execute_command timed out ({}s) for: {}",
                execute_command_timeout_sec,
                cmd);
            timed_out = true;
            break;
        }

        if (poll_ret < 0)
        {
            diag_log_cmd("poll ERROR", cmd);
            break;
        }

        ssize_t n =
            ::read(pipe_fds[0], buffer.data(), buffer.size());
        if (n <= 0)
        {
            diag_log_cmd("read EOF/error", cmd);
            break;
        }
        result.append(buffer.data(), static_cast<size_t>(n));
    }

    ::close(pipe_fds[0]);

    if (timed_out)
    {
        ::kill(pid, SIGKILL);
        diag_log_cmd("killed hung child", cmd);
        spdlog::info("killed hung child pid {}", pid);
    }

    diag_log_cmd("before waitpid", cmd);
    ::waitpid(pid, nullptr, 0);
    diag_log_cmd("after waitpid, returning", cmd);

    return result;
}

/// Communicate with wpa_supplicant directly via its control socket.
/// This avoids fork/exec entirely — pure socket I/O that works from
/// any thread without deadlock risk.
std::string
wpa_cli(std::string const& interface, std::string const& cmd)
{
    diag_log_cmd("wpa_cli enter (socket)", cmd.c_str());

    // wpa_supplicant control protocol uses uppercase command names
    // (e.g., SCAN not scan). Convert the first word to uppercase.
    std::string upper_cmd = cmd;
    auto space_pos = upper_cmd.find(' ');
    auto cmd_end = (space_pos != std::string::npos)
        ? space_pos : upper_cmd.size();
    std::transform(
        upper_cmd.begin(),
        upper_cmd.begin() + static_cast<std::ptrdiff_t>(cmd_end),
        upper_cmd.begin(),
        [](unsigned char c) { return std::toupper(c); });

    // wpa_supplicant control socket path
    std::string ctrl_path =
        "/var/run/wpa_supplicant/" + interface;

    // Local socket path (unique per call using thread id + timestamp)
    auto tid = ::pthread_self();
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    std::string local_path =
        "/tmp/wpa_ctrl_" + std::to_string(tid) + "_" +
        std::to_string(ts.tv_nsec);

    int sock = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        diag_log("wpa_cli: socket() failed");
        return {};
    }

    // Bind local end
    struct sockaddr_un local_addr{};
    local_addr.sun_family = AF_UNIX;
    ::strncpy(
        local_addr.sun_path,
        local_path.c_str(),
        sizeof(local_addr.sun_path) - 1);

    if (::bind(
            sock,
            reinterpret_cast<struct sockaddr*>(&local_addr),
            sizeof(local_addr)) < 0)
    {
        diag_log("wpa_cli: bind() failed");
        ::close(sock);
        ::unlink(local_path.c_str());
        return {};
    }

    // Connect to wpa_supplicant
    struct sockaddr_un dest_addr{};
    dest_addr.sun_family = AF_UNIX;
    ::strncpy(
        dest_addr.sun_path,
        ctrl_path.c_str(),
        sizeof(dest_addr.sun_path) - 1);

    if (::connect(
            sock,
            reinterpret_cast<struct sockaddr*>(&dest_addr),
            sizeof(dest_addr)) < 0)
    {
        diag_log_cmd("wpa_cli: connect() failed to", ctrl_path.c_str());
        ::close(sock);
        ::unlink(local_path.c_str());
        return {};
    }

    diag_log_cmd("wpa_cli: sending command", upper_cmd.c_str());

    // Send command
    if (::send(sock, upper_cmd.c_str(), upper_cmd.size(), 0) < 0)
    {
        diag_log("wpa_cli: send() failed");
        ::close(sock);
        ::unlink(local_path.c_str());
        return {};
    }

    // Wait for response with timeout
    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN;

    std::string result;
    int poll_ret = ::poll(&pfd, 1, 10000); // 10 second timeout
    if (poll_ret > 0)
    {
        std::array<char, 4096> buf;
        ssize_t n = ::recv(sock, buf.data(), buf.size() - 1, 0);
        if (n > 0)
        {
            buf[static_cast<size_t>(n)] = '\0';
            result = buf.data();
        }
        diag_log_cmd("wpa_cli: got response", result.c_str());
    }
    else if (poll_ret == 0)
    {
        diag_log_cmd("wpa_cli: timeout waiting for response", cmd.c_str());
    }
    else
    {
        diag_log("wpa_cli: poll() error");
    }

    ::close(sock);
    ::unlink(local_path.c_str());

    return result;
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
    // std::unique only works on adjacent elements, so use a set
    std::unordered_set<std::string> seen;
    auto it = std::remove_if(
        networks.begin(),
        networks.end(),
        [&seen](wifi_network const& net) {
            return !seen.insert(net.ssid).second;
        });
    networks.erase(it, networks.end());

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
    wpa_cli(interface, "set_network " + id_str + " ssid \"" + ssid + "\"");

    // Set security
    if (security == wifi_security::open || password.empty())
    {
        wpa_cli(interface, "set_network " + id_str + " key_mgmt NONE");
    }
    else
    {
        wpa_cli(interface, "set_network " + id_str + " psk \"" + password + "\"");
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
            auto prev_status = connection_status;
            connection_status = new_status;

            // Start DHCP when transitioning to connected.
            // Must use std::system with & to avoid blocking:
            // execute_command (popen) blocks until the child closes
            // stdout, but udhcpc runs as a long-lived daemon and
            // never exits, so popen would block forever.
            if (new_status == wifi_connection_status::connected &&
                prev_status != wifi_connection_status::connected)
            {
                spdlog::info("Starting udhcpc on {}", interface);
                std::string dhcp_cmd =
                    "udhcpc -i " + interface +
                    " -p /var/run/udhcpc." + interface + ".pid"
                    " -t 5 -T 2 -S"
                    " >/dev/null 2>&1 &";
                [[maybe_unused]] auto rc = std::system(dhcp_cmd.c_str());
            }

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
    diag_log("scan_networks: after spdlog::debug, before wpa_cli");

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
            // udhcpc started by update_connection_status() in background.
            // IP polling is handled by the caller (NetworkSettings QTimer).
            result.success = true;
            result.ip_address = ip_address(); // may be empty if DHCP pending

            if (remember)
            {
                wpa_cli(m_impl->interface, "save_config");
            }

            spdlog::info("Connected to {} (IP: {})", ssid,
                result.ip_address.empty() ? "pending" : result.ip_address);
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
    [[maybe_unused]] auto rc = std::system("killall -q udhcpc 2>/dev/null");
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
