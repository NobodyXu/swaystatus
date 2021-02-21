#include <err.h>

#include <atomic>

#include <NetworkManager.h>

#include "printer.hpp"
#include "NMIPConfig.hpp"
#include "print_network_interfaces.h"

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

void print_network_interfaces()
{
    if (!nm_client_networking_get_enabled(client)) {
        print_literal_str("Network disabled");
        return;
    }
    if (connectivity_state == NM_CONNECTIVITY_NONE) {
        print_literal_str("Not connected");
        return;
    }

    if (seconds++ == 120) {
        /*
         * Check connectivity every 120 seconds
         */
        nm_client_check_connectivity_async(client, NULL, set_connectivity, NULL);
        seconds = 0;
    }

    NMActiveConnection * const conn = nm_client_get_primary_connection(client);
    if (!conn) {
        print_literal_str("Unrecongnizable device");
        return;
    }

    NMIPConfig * const ipv4_config = nm_active_connection_get_ip4_config(conn);
    NMIPConfig * const ipv6_config = nm_active_connection_get_ip6_config(conn);
    if (ipv4_config == NULL || ipv6_config == NULL) {
        print_literal_str("Error: Current activate connection get deactivated");
        return;
    }

    swaystatus::print(
        format,
        fmt::arg("connectivity_state", connectivity_state.load()),
        fmt::arg("ipv4_config", ipv4_config),
        fmt::arg("ipv6_config", ipv6_config)
    );
}
}
