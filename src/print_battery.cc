#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <upower.h>

#include "printer.hpp"
#include "Conditional.hpp"
#include "print_battery.h"

using swaystatus::Conditional;

static const char *format;
static UpClient *client;

extern "C" {
void init_upclient(const char *format_str)
{
    format = format_str;

    GError *error = NULL;
    client = up_client_new_full(NULL, &error);
    if (client == NULL)
        errx(1, "%s failed: %s", "up_client_new", error->message);
}

void print_battery()
{
    UpDevice *device = up_client_get_display_device(client);

    UpDeviceState state;
    gdouble percentage;
    gdouble temperature;
    g_object_get(device, 
        "state", &state,
        "percentage", &percentage,
        "temperature", &temperature,
    NULL);

    swaystatus::print(
        format,
        fmt::arg("state", up_device_state_to_string(state)),
        fmt::arg("level", static_cast<unsigned>(percentage)),
        fmt::arg("temperature", temperature),
        fmt::arg("is_fully_charged", Conditional{state == UP_DEVICE_STATE_FULLY_CHARGED}),
        fmt::arg("is_discharging",   Conditional{state == UP_DEVICE_STATE_DISCHARGING}),
        fmt::arg("is_charging",      Conditional{state == UP_DEVICE_STATE_CHARGING}),
        fmt::arg("is_empty",         Conditional{state == UP_DEVICE_STATE_EMPTY})
    );

    g_object_unref(device);
}
}
