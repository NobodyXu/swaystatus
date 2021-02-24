#include <err.h>

#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "printer.hpp"
#include "Conditional.hpp"
#include "networking.hpp"
#include "print_network_interfaces.h"

using swaystatus::Conditional;
using swaystatus::interface_stats;
using swaystatus::Interfaces;

static const char *format;

static uint32_t cycle_cnt;
static uint32_t interval;

static Interfaces interfaces;

extern "C" {
static void getifaddrs_checked()
{
    interfaces.clear();

    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) < 0)
        err(1, "%s failed", "getifaddrs");

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
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

        auto *interface = interfaces[ifa->ifa_name];
        if (!interface)
            break;
        interface->flags = ifa->ifa_flags;
        switch (ifa->ifa_addr->sa_family) {
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
void init_network_interfaces_scanning(const char *format_str, uint32_t interval_arg)
{
    format = format_str;
    interval = interval_arg;

    getifaddrs_checked();
}

void print_network_interfaces()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;
        getifaddrs_checked();
    }

    swaystatus::print(
        format,
        fmt::arg("is_not_connected",      Conditional{interfaces.is_empty()}),
        fmt::arg("is_connected",          Conditional{!interfaces.is_empty()}),
        fmt::arg("per_interface_fmt_str", interfaces)
    );
}
}
