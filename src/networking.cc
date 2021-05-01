#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#include <err.h>

#include <cstring>
#include <cinttypes>
#include <climits>
#include <cerrno>
#include <algorithm>

#include "utility.h"
#include "formatting/fmt_utility.hpp"
#include "formatting/Conditional.hpp"
#include "mem_size_t.hpp"
#include "networking.hpp"

#include "formatting/fmt/include/fmt/core.h"

using swaystatus::Conditional;
using swaystatus::mem_size_t;
using swaystatus::find_end_of_format;

namespace swaystatus {
void ipv4_addrs::add(const struct sockaddr *src_addr) noexcept
{
    if (cnt == array.size())
        return;

    ++cnt;
    auto &dest_ipv4_addr = array[cnt - 1];
    auto &src_ipv4_addr = *reinterpret_cast<const struct sockaddr_in*>(src_addr);

    dest_ipv4_addr = src_ipv4_addr.sin_addr;
}
void ipv4_addrs::reset() noexcept
{
    cnt = 0;
}
auto ipv4_addrs::begin() const noexcept -> const_iterator
{
    return array.begin();
}
auto ipv4_addrs::end() const noexcept -> const_iterator
{
    return begin() + cnt;
}
void ipv6_addrs::add(const struct sockaddr *src_addr) noexcept
{
    if (cnt == array.size())
        return;

    ++cnt;
    auto &dest_ipv6_addr = array[cnt - 1];
    auto &src_ipv6_addr = *reinterpret_cast<const struct sockaddr_in6*>(src_addr);

    dest_ipv6_addr = src_ipv6_addr.sin6_addr;
}
void ipv6_addrs::reset() noexcept
{
    cnt = 0;
}
auto ipv6_addrs::begin() const noexcept -> const_iterator
{
    return array.begin();
}
auto ipv6_addrs::end() const noexcept -> const_iterator
{
    return begin() + cnt;
}

interface_stats Interface::get_empty_stats() noexcept
{
    interface_stats stats;
    std::memset(&stats, 0, sizeof(stats));
    return stats;
}

bool Interface::operator == (std::string_view interface_name) const noexcept
{
    return name == interface_name;
}
bool Interface::operator != (std::string_view interface_name) const noexcept
{
    return !(*this == interface_name);
}

void Interface::reset() noexcept
{
    /*
     * name and flags will be overwriten anyway, so it's ok to not reset them.
     */
    stat = get_empty_stats();
    ipv4_addrs_v.reset();
    ipv6_addrs_v.reset();
}

auto Interfaces::operator [] (std::string_view name) noexcept -> iterator
{
    auto it = std::find(interfaces.begin(), interfaces.begin() + cnt, name);
    if (it != interfaces.begin() + cnt)
        return it;

    if (cnt == interfaces.size())
        return end();

    ++cnt;
    auto &interface = interfaces[cnt - 1];
    interface.name = name;

    return end() - 1;
}

bool Interfaces::is_empty() const noexcept
{
    return cnt == 0;
}
auto Interfaces::size() const noexcept -> std::size_t
{
    return cnt;
}

auto Interfaces::begin() noexcept -> iterator
{
    return interfaces.begin();
}
auto Interfaces::end() noexcept -> iterator
{
    return begin() + cnt;
}

auto Interfaces::begin() const noexcept -> const_iterator
{
    return interfaces.begin();
}
auto Interfaces::end() const noexcept -> const_iterator
{
    return begin() + cnt;
}

auto Interfaces::cbegin() const noexcept -> const_iterator
{
    return interfaces.cbegin();
}
auto Interfaces::cend() const noexcept -> const_iterator
{
    return cbegin() + cnt;
}

void Interfaces::clear() noexcept
{
    for (auto &interface: *this)
        interface.reset();

    cnt = 0;
}
void Interfaces::update()
{
    clear();
    
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) < 0)
        err(1, "%s failed", "getifaddrs");
    
    for (auto *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        // Filter out uninterested entries
        if (ifa->ifa_addr == nullptr)
            continue;
    
        auto ifa_flags = ifa->ifa_flags;
        if (ifa_flags & IFF_LOOPBACK)
            continue;
        if (!(ifa_flags & IFF_UP))
            continue;
        if (!(ifa_flags & IFF_RUNNING))
            continue;
    
        auto sa_family = ifa->ifa_addr->sa_family;
        if (sa_family != AF_INET && sa_family != AF_INET6 && sa_family != AF_PACKET)
            continue;

        // Add new interface
        auto *interface = (*this)[ifa->ifa_name];
        if (!interface) // If it is full, then stop getting more
            break;
        interface->flags = ifa->ifa_flags;
        switch (sa_family) {
            case AF_INET:
                interface->ipv4_addrs_v.add(ifa->ifa_addr);
                break;
    
            case AF_INET6:
                interface->ipv6_addrs_v.add(ifa->ifa_addr);
                break;
    
            case AF_PACKET:
                interface->stat = *static_cast<interface_stats*>(ifa->ifa_data);
                break;
        }
    }
    
    freeifaddrs(ifaddr);
}
} /* namespace swaystatus */

using Interfaces_formatter = fmt::formatter<swaystatus::Interfaces>;

auto Interfaces_formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    end = find_end_of_format(ctx);

    fmt_str = std::string_view{it, static_cast<std::size_t>(end - it)};

    return end;
}
auto Interfaces_formatter::format(const Interfaces &interfaces, format_context &ctx) ->
    format_context_it
{
    if (fmt_str.size() != 0) {
        auto out = ctx.out();

        std::size_t i = 0;
        for (const auto &interface: interfaces) {
            out = format_to(
                out,
                fmt_str,
                fmt::arg("name", interface.name),
#define FMT_FLAGS(name, var) \
    fmt::arg(name, Conditional{static_cast<bool>(interface.flags & IFF_##var)})
                FMT_FLAGS("has_broadcast_support", BROADCAST),
                FMT_FLAGS("is_pointopoint",        POINTOPOINT),
                FMT_FLAGS("has_no_arp_support",    NOARP),
                FMT_FLAGS("is_in_promisc_mode",    PROMISC),
                FMT_FLAGS("is_in_notrailers_mode", NOTRAILERS),
                FMT_FLAGS("is_master",             MASTER),
                FMT_FLAGS("is_slave",              SLAVE),
                FMT_FLAGS("has_multicast_support", MULTICAST),
                FMT_FLAGS("has_portsel_support",   PORTSEL),
                FMT_FLAGS("is_automedia_active",   AUTOMEDIA),
                FMT_FLAGS("is_dhcp",               DYNAMIC),
                
                fmt::arg(
                    "HAS_UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO", 
# if  __UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO
                    Conditional{true}
                ),
                FMT_FLAGS("is_lower_up",           LOWER_UP),
                FMT_FLAGS("is_dormant",            DORMANT),
                FMT_FLAGS("is_echo_device",        ECHO),
# endif
                    Conditional{false}
                ),
#undef  FMT_FLAGS
#define FMT_STAT(attr) fmt::arg(# attr, interface.stat.attr)
	            FMT_STAT(rx_packets),
	            FMT_STAT(tx_packets),

	            fmt::arg("rx_bytes", mem_size_t{interface.stat.rx_bytes}),
	            fmt::arg("tx_bytes", mem_size_t{interface.stat.tx_bytes}),

	            FMT_STAT(rx_errors),
	            FMT_STAT(tx_errors),
	            FMT_STAT(rx_dropped),
	            FMT_STAT(tx_dropped),
	            FMT_STAT(multicast),
	            FMT_STAT(collisions),

	            FMT_STAT(rx_length_errors),
	            FMT_STAT(rx_over_errors),
	            FMT_STAT(rx_crc_errors),
	            FMT_STAT(rx_frame_errors),
	            FMT_STAT(rx_fifo_errors),
	            FMT_STAT(rx_missed_errors),

	            FMT_STAT(tx_aborted_errors),
	            FMT_STAT(tx_carrier_errors),
	            FMT_STAT(tx_fifo_errors),
	            FMT_STAT(tx_heartbeat_errors),
	            FMT_STAT(tx_window_errors),

	            FMT_STAT(rx_compressed),
	            FMT_STAT(tx_compressed),
#undef  FMT_STAT
                fmt::arg("ipv4_addrs", interface.ipv4_addrs_v),
                fmt::arg("ipv6_addrs", interface.ipv6_addrs_v)
            );

            if (++i != interfaces.size()) {
                *out = ' ';
                ++out;
            }
        }

        return out;
    } else
        return ctx.out();
}

static auto parse_limit(fmt::format_parse_context &ctx, std::size_t *limit)
{
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    int base = 10;
    if (*it == '0' && (it + 1) != end && *(it + 1) == 'x')
        base = 16;

    errno = 0;
    char *endptr;
    uintmax_t result = strtoumax(it, &endptr, base);
    if (errno == ERANGE || result > SIZE_MAX)
        FMT_THROW(fmt::format_error("invalid format: Integer too big"));
    if (endptr == end || *endptr != '}')
        FMT_THROW(fmt::format_error("invalid format: Unterminated '{'"));

    *limit = result;

    return static_cast<decltype(it)>(endptr);
}
template <class It>
static It format_to(It out, int af, const void *addr, std::size_t buffer_sz)
{
    char buffer[buffer_sz];

    FMT_ASSERT(
        inet_ntop(af, addr, buffer, buffer_sz) != nullptr,
        std::strerror(errno)
    );

    return fmt::format_to(out, "{}", static_cast<const char*>(buffer));
}

using ipv4_addrs_formatter = fmt::formatter<swaystatus::ipv4_addrs>;

auto ipv4_addrs_formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    return parse_limit(ctx, &limit);
}
auto ipv4_addrs_formatter::format(const ipv4_addrs &addrs, format_context &ctx) ->
    format_context_it
{
    auto out = ctx.out();

    std::size_t cnt = 0;
    for (auto &addr: addrs) {
        out = format_to(out, AF_INET, &addr, INET_ADDRSTRLEN);

        ++cnt;
        if (cnt == limit)
            break;
        if (cnt != addrs.cnt) {
            *out = ' ';
            ++out;
        }
    }

    return out;
}

using ipv6_addrs_formatter = fmt::formatter<swaystatus::ipv6_addrs>;

auto ipv6_addrs_formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    return parse_limit(ctx, &limit);
}
auto ipv6_addrs_formatter::format(const ipv6_addrs &addrs, format_context &ctx) ->
    format_context_it
{
    auto out = ctx.out();

    std::size_t cnt = 0;
    for (auto &addr: addrs) {
        out = format_to(out, AF_INET6, &addr, INET6_ADDRSTRLEN);

        ++cnt;
        if (cnt == limit)
            break;
        if (cnt != addrs.cnt) {
            *out = ' ';
            ++out;
        }
    }

    return out;
}
