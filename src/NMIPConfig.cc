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
            return "";
    }
}
auto ConnStateformatter::format(const NMConnectivityState &state, format_context &ctx) ->
    format_context_it
{
    return format_to(ctx.out(), "{}", connectivity2str(state));
}

using IPConfigformatter = fmt::formatter<NMIPConfig*>;

auto IPConfigformatter::format(NMIPConfig *ipconfig, format_context &ctx) -> format_context_it
{
    GPtrArray *addresses = nm_ip_config_get_addresses(ipconfig);

    auto out = ctx.out();
    g_ptr_array_foreach(
        addresses,
        [](gpointer data, gpointer user_data) noexcept
        {
            auto &out = *static_cast<format_context_it*>(user_data);
            const char *address = nm_ip_address_get_address(static_cast<NMIPAddress*>(data));

            out = format_to(out, "{} ", address);
        }, 
        &out
    );

    return out;
}
