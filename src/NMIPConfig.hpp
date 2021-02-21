#ifndef  __swaystatus_NetworkAddresses_H__
# define __swaystatus_NetworkAddresses_H__

# include <cstddef>
# include <string_view>

# include <NetworkManager.h>

# include "dep/fmt/include/fmt/format.h"

template <>
struct fmt::formatter<NMConnectivityState>: fmt::formatter<std::string_view> {
    using format_context_it = typename format_context::iterator;

    auto format(const NMConnectivityState &state, format_context &ctx) -> format_context_it;
};

template <>
struct fmt::formatter<NMIPConfig*>: fmt::formatter<std::string_view> {
    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::size_t cnt = -1;

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(NMIPConfig *ipconfig, format_context &ctx) -> format_context_it;
};

#endif
