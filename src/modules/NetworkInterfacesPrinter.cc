#include <err.h>

#include <sys/types.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "../formatting/Conditional.hpp"
#include "../networking.hpp"

#include "NetworkInterfacesPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class NetworkInterfacesPrinter: public Base {
    Interfaces interfaces;

    void getifaddrs_checked()
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

public:
    NetworkInterfacesPrinter(void *config):
        Base{
            config, "NetworkInterfacesPrinter"sv,
            60 * 2,
            "{is_connected:{per_interface_fmt_str:"
                "{name} {is_dhcp:DHCP }in: {rx_bytes} out: {tx_bytes} "
                "{ipv4_addrs:1} {ipv6_addrs:1}"
            "}}",
            "{is_connected:{per_interface_fmt_str:"
                "{name}"
            "}}"
        }
    {}

    void update()
    {
        getifaddrs_checked();
    }
    void do_print(const char *format)
    {
        print(
            format,
            fmt::arg("is_not_connected",      Conditional{interfaces.is_empty()}),
            fmt::arg("is_connected",          Conditional{!interfaces.is_empty()}),
            fmt::arg("per_interface_fmt_str", interfaces)
        );
    }
};

std::unique_ptr<Base> makeNetworkInterfacesPrinter(void *config)
{
    return std::make_unique<NetworkInterfacesPrinter>(config);
}
} /* namespace swaystatus::modules */
