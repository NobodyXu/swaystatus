#ifndef  __swaystatus_NetworkAddresses_H__
# define __swaystatus_NetworkAddresses_H__

# include "fmt_config.h"

# include <cstddef>
# include <string_view>

# include <NetworkManager.h>

# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
struct IPConfig {
    NMIPConfig *ipconfig;
};
}

template <>
struct fmt::formatter<NMConnectivityState>: fmt::formatter<std::string_view> {
    using format_context_it = typename format_context::iterator;

    auto format(const NMConnectivityState &state, format_context &ctx) -> format_context_it;
};

template <>
struct fmt::formatter<swaystatus::IPConfig>: fmt::formatter<std::string_view> {
    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    using IPConfig = swaystatus::IPConfig;

    std::size_t cnt = -1;

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const IPConfig &config, format_context &ctx) -> format_context_it;
};

#endif
