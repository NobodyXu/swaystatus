#include <err.h>
#include <atomic>

#include <NetworkManager.h>

#include "printer.hpp"
#include "Conditional.hpp"
#include "NMIPConfig.hpp"
#include "print_network_interfaces.h"

using swaystatus::Conditional;
using swaystatus::IPConfig;

static NMClient *client;
static std::atomic<NMConnectivityState> connectivity_state;
static unsigned seconds;
static const char *format;

extern "C" {
void init_network_interfaces_scanning(const char *format_str)
{
    format = format_str;

    GError *error = NULL;
    client = nm_client_new(NULL, &error);
    if (client == NULL)
        errx(1, "%s failed: %s", "nm_client_new", error->message);

    if (nm_client_networking_get_enabled(client))
        connectivity_state = nm_client_get_connectivity(client);
    else
        connectivity_state = NM_CONNECTIVITY_UNKNOWN;
}

static void set_connectivity(GObject *src, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    connectivity_state = nm_client_check_connectivity_finish(client, res, &error);
    if (error)
        errx(1, "%s failed: %s", "nm_client_check_connectivity_finish", error->message);
}

void retrieve_info(NMActiveConnection **conn, NMIPConfig **ipv4_config, NMIPConfig **ipv6_config)
{
    if (connectivity_state == NM_CONNECTIVITY_NONE)
        return;

    *conn = nm_client_get_primary_connection(client);
    if (!(*conn))
        return;

    *ipv4_config = nm_active_connection_get_ip4_config(*conn);
    *ipv6_config = nm_active_connection_get_ip6_config(*conn);
}
void print_network_interfaces()
{
    NMActiveConnection *conn = NULL;
    NMIPConfig *ipv4_config = NULL;
    NMIPConfig *ipv6_config = NULL;

    if (seconds++ == 120) {
        /*
         * Check connectivity every 120 seconds
         */
        nm_client_check_connectivity_async(client, NULL, set_connectivity, NULL);
        seconds = 0;
    }

    bool is_network_enabled = true;
    if (nm_client_networking_get_enabled(client))
        retrieve_info(&conn, &ipv4_config, &ipv6_config);
    else
        is_network_enabled = false;

    if (ipv4_config == NULL || ipv6_config == NULL) {
        conn = NULL;
        ipv4_config = NULL;
        ipv6_config = NULL;
    }

    auto state = connectivity_state.load();

    swaystatus::print(
        format,
        fmt::arg("is_network_enabled",       Conditional{is_network_enabled}),
        fmt::arg("is_not_network_enabled",   Conditional{!is_network_enabled}),
        fmt::arg("is_network_disabled",   Conditional{!is_network_enabled}),

        fmt::arg("has_active_connection",    Conditional{conn != NULL}),
        fmt::arg("has_no_active_connection", Conditional{conn == NULL}),

        fmt::arg("has_no_connection",          Conditional{state == NM_CONNECTIVITY_NONE}),
        fmt::arg("has_connection",             Conditional{state != NM_CONNECTIVITY_NONE}),
        fmt::arg("has_full_connection",        Conditional{state == NM_CONNECTIVITY_FULL}),
        fmt::arg("has_limited_connection",     Conditional{state == NM_CONNECTIVITY_LIMITED}),
        fmt::arg("has_portal_connection",      Conditional{state == NM_CONNECTIVITY_PORTAL}),

        fmt::arg("connectivity_state", state),
        fmt::arg("ipv4_config", IPConfig{ipv4_config}),
        fmt::arg("ipv6_config", IPConfig{ipv6_config})
    );
}
}
