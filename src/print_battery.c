#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <upower.h>

#include "print_battery.h"

void init_print_battery(struct Print_Battery_Data *data)
{
    GError *error = NULL;
    data->upclient = up_client_new_full(NULL, &error);
    if (data->upclient == NULL)
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

void print_battery(const struct Print_Battery_Data *data)
{
    UpClient *client = data->upclient;

    UpDevice *device = up_client_get_display_device(client);

    UpDeviceState state;
    gdouble percentage;
    g_object_get(device, "state", &state, "percentage", &percentage, NULL);

    printf("BAT: %s %u%%", state2str(state), (unsigned) percentage);
}
