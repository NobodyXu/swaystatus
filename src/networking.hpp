#ifndef  __swaystatus_networking_H__
# define __swaystatus_networking_H__

# include "formatting/fmt_config.hpp"

# include <sys/types.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <linux/if_link.h>

# include <cstddef>
# include <string>
# include <string_view>
# include <array>

# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
using ipv4_addr = struct in_addr;
using ipv6_addr = struct in6_addr;

using interface_stats = struct rtnl_link_stats;

struct ip_addrs {};
/**
 * It is unlikely for one computer to have more than 8 addresses.
 */
struct ipv4_addrs: ip_addrs {
    using const_iterator = typename std::array<ipv4_addr, 8>::const_iterator;

    std::size_t cnt = 0;
    std::array<ipv4_addr, 8> array;

    void add(const struct sockaddr *src_addr) noexcept;
    void reset() noexcept;

    auto begin() const noexcept -> const_iterator;
    auto end() const noexcept -> const_iterator;
};
struct ipv6_addrs: ip_addrs {
    using const_iterator = typename std::array<ipv6_addr, 8>::const_iterator;

    std::size_t cnt = 0;
    std::array<ipv6_addr, 8> array;

    void add(const struct sockaddr *src_addr) noexcept;
    void reset() noexcept;

    auto begin() const noexcept -> const_iterator;
    auto end() const noexcept -> const_iterator;
};
struct Interface {
    static interface_stats get_empty_stats() noexcept;

    std::string name;
    unsigned int flags;   /* Flags from SIOCGIFFLAGS */

    interface_stats stat = get_empty_stats();

    ipv4_addrs ipv4_addrs_v;
    ipv6_addrs ipv6_addrs_v;

    /**
     * operator == and != are used to find the interface.
     */
    bool operator == (std::string_view interface_name) const noexcept;
    bool operator != (std::string_view interface_name) const noexcept;

    void reset() noexcept;
};

class Interfaces {
    std::size_t cnt = 0;
    /**
     * It is unlikely for one computer to have more than 8 network interfaces.
     */
    std::array<Interface, 8> interfaces;

public:
    using iterator       = typename std::array<Interface, 8>::iterator;
    using const_iterator = typename std::array<Interface, 8>::const_iterator;

    Interfaces() = default;
    ~Interfaces() = default;

    auto operator [] (std::string_view name) noexcept -> iterator;

    bool is_empty() const noexcept;
    auto size() const noexcept -> std::size_t;

    auto begin() noexcept -> iterator;
    auto end() noexcept -> iterator;

    auto begin() const noexcept -> const_iterator;
    auto end() const noexcept -> const_iterator;

    auto cbegin() const noexcept -> const_iterator;
    auto cend() const noexcept -> const_iterator;

    void clear() noexcept;
};
}

template <>
struct fmt::formatter<swaystatus::Interfaces>
{
    using Interfaces = swaystatus::Interfaces;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::string_view fmt_str = "";

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const Interfaces &interfaces, format_context &ctx) -> format_context_it;
};

template <>
struct fmt::formatter<swaystatus::ipv4_addrs>
{
    using ipv4_addrs = swaystatus::ipv4_addrs;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::size_t limit = -1;

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const ipv4_addrs &addrs, format_context &ctx) -> format_context_it;
};

template <>
struct fmt::formatter<swaystatus::ipv6_addrs>
{
    using ipv6_addrs = swaystatus::ipv6_addrs;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::size_t limit = -1;

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const ipv6_addrs &addrs, format_context &ctx) -> format_context_it;
};

#endif
