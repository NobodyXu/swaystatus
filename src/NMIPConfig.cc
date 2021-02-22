#include <cerrno>
#include <cinttypes>

#include <algorithm>

#include "NMIPConfig.hpp"

using ConnStateformatter = fmt::formatter<NMConnectivityState>;

static const char* connectivity2str(NMConnectivityState state)
{
    switch (state) {
        case NM_CONNECTIVITY_UNKNOWN:
            return "Unknown Connectivity";

        case NM_CONNECTIVITY_NONE:
            return "Not Connected";

        case NM_CONNECTIVITY_PORTAL:
            return "Hijacked by Portal";

        case NM_CONNECTIVITY_LIMITED:
            return "Limited Connection";

        case NM_CONNECTIVITY_FULL:
            return "Full Connection";
    }
}
auto ConnStateformatter::format(const NMConnectivityState &state, format_context &ctx) ->
    format_context_it
{
    return format_to(ctx.out(), "{}", connectivity2str(state));
}

using IPConfigformatter = fmt::formatter<swaystatus::IPConfig>;

static auto strtosize(const char **it, const char *end)
{
    char *endptr;
    errno = 0;
    uintmax_t result = strtoumax(*it, &endptr, 10);
    if (endptr == end || (*endptr != ' ' && *endptr != '}'))
        FMT_THROW(fmt::format_error("invalid format"));
    if (errno == ERANGE || result > static_cast<std::size_t>(-1))
        FMT_THROW(fmt::format_error("invalid format: count out of range"));

    *it = endptr;

    return static_cast<std::size_t>(result);
}
auto IPConfigformatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    cnt = strtosize(&it, end);

    if (*it == '}')
        return it;
    ++it;

    ctx.advance_to(it);
    return fmt::formatter<std::string_view>::parse(ctx);
}
auto IPConfigformatter::format(const IPConfig &config, format_context &ctx) -> format_context_it
{
    auto *ipconfig = config.ipconfig;

    auto out = ctx.out();

    if (ipconfig == nullptr)
        return out;

    GPtrArray *addresses = nm_ip_config_get_addresses(ipconfig);

    const size_t end = std::min(cnt, static_cast<std::size_t>(addresses->len));
    for (size_t i = 0; i != end; ++i) {
        auto *address = static_cast<NMIPAddress*>(g_ptr_array_index(addresses, i));
        auto address_str = std::string_view{nm_ip_address_get_address(address)};

        out = formatter<std::string_view>::format(address_str, ctx);
        if (i + 1 != end) {
            *out = ' ';
            ++out;
            
            ctx.advance_to(out);
        }
    }

    return out;
}
