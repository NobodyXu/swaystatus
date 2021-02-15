#include <stdio.h>
#include <err.h>

#include <NetworkManager.h>

#include "print_network_interfaces.h"

static NMClient *client;
static NMConnectivityState connectivity_state;
static unsigned cnt;

void init_network_interfaces_scanning()
{
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
static const char* connectivity2str(NMConnectivityState state)
{
    switch (nm_client_get_connectivity(client)) {
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

static void print_address(gpointer data, gpointer user_data)
{
    const char *address = nm_ip_address_get_address((NMIPAddress*) data);

    printf("%s ", address);
}
static void print_addresses(NMIPConfig *config)
{
    GPtrArray *addresses = nm_ip_config_get_addresses(config);
    g_ptr_array_foreach(addresses, print_address, NULL);
}

void print_network_interfaces()
{
    if (!nm_client_networking_get_enabled(client)) {
        fputs("Network disabled", stdout);
        return;
    }
    if (connectivity_state == NM_CONNECTIVITY_NONE) {
        fputs("Not connected", stdout);
        return;
    }

    const char *connectivity = connectivity2str(connectivity_state);
    if (cnt++ == 5)
        nm_client_check_connectivity_async(client, NULL, set_connectivity, NULL);

    NMActiveConnection * const conn = nm_client_get_primary_connection(client);
    if (!conn) {
        fputs("Unrecongnizable device", stdout);
        return;
    }

    NMIPConfig * const ipv4_config = nm_active_connection_get_ip4_config(conn);
    NMIPConfig * const ipv6_config = nm_active_connection_get_ip6_config(conn);
    if (ipv4_config == NULL || ipv6_config == NULL) {
        fputs("Error: Current activate connection get deactivated", stdout);
        return;
    }

    fputs(connectivity, stdout);

    print_addresses(ipv4_config);
    print_addresses(ipv6_config);
}