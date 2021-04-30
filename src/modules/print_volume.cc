#include <err.h>
#include <exception>

#include "../alsa.h"
#include "../process_configuration.h"
#include "../handle_click_events.h"
#include "../formatting/printer.hpp"
#include "print_volume.h"

using swaystatus::print;
using swaystatus::get_user_specified_property_str;

static const char * const module_name = "volume";

static uint32_t cycle_cnt;
static uint32_t interval;

static const char *user_specified_properties_str;

static const char *full_text_format;
static const char *short_text_format;

extern "C" {
void init_volume_monitor(void *config)
{
    full_text_format = get_format(config, "vol {volume}%");
    short_text_format = get_short_format(config, NULL);

    interval = get_update_interval(config, "volume", 1);

    initialize_alsa_lib(
        get_property(config, "mix_name", "Master"),
        get_property(config, "card",     "default")
    );

    add_click_event_handler(module_name, get_click_event_handler(config));

    user_specified_properties_str = get_user_specified_property_str(config, "mix_name", "card");
}

static void print_fmt(const char *name, const char *format)
{
    print("\"{}\":\"", name);

    try {
        print(format, fmt::arg("volume", get_audio_volume()));
    } catch (const std::exception &e) {
        errx(1, "Failed to print %s format in print_%s.cc: %s", name, "volume", e.what());
    }

    print_literal_str("\",");
}
void print_volume()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;
        update_volume();
    }

    print_literal_str("{\"name\":\"");
    print_str(module_name);
    print_literal_str("\",\"instance\":\"0\",");

    print_fmt("full_text", full_text_format);
    if (short_text_format)
        print_fmt("short_text", short_text_format);

    print_str(user_specified_properties_str);

    print_literal_str("},");
}
} /* extern "C" */
