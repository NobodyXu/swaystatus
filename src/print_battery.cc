#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <upower.h>

#include "printer.hpp"
#include "print_battery.h"

static UpClient *client;

extern "C" {
void init_upclient()
{
    GError *error = NULL;
    client = up_client_new_full(NULL, &error);
    if (client == NULL)
        errx(1, "up_client_new failed: %s", error->message);
}

static const char* state2str(UpDeviceState state)
{
    switch (state) {
        default:
        case 0:
            return "Unknown";
        case 1:
            return "Charging";
        case 2:
            return "Discharging";
        case 3:
            return "Empty";
        case 4:
            return "Fully charged";
        case 5:
            return "Pending charge";
        case 6:
            return "Pending discharge";
    }
}

void print_battery()
{
    UpDevice *device = up_client_get_display_device(client);

    UpDeviceState state;
    gdouble percentage;
    g_object_get(device, "state", &state, "percentage", &percentage, NULL);

    swaystatus::print("{} {}%", state2str(state), (unsigned) percentage);

    g_object_unref(device);
}
}
