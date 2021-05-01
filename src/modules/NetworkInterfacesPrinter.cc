#include "../formatting/Conditional.hpp"
#include "../networking.hpp"

#include "NetworkInterfacesPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class NetworkInterfacesPrinter: public Base {
    Interfaces interfaces;

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
        interfaces.update();
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
