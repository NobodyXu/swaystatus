#include <err.h>
#include <exception>

#include "alsa.h"
#include "process_configuration.h"
#include "printer.hpp"
#include "print_volume.h"

using swaystatus::print;

static uint32_t cycle_cnt;
static uint32_t interval;

extern "C" {
static const char *full_text_format;
static const char *short_text_format;

void init_volume_monitor(const void *config)
{
    full_text_format = get_format(config, "volume", "vol {volume}%");
    short_text_format = get_short_format(config, "volume", NULL);

    interval = get_update_interval(config, "volume", 1);

    initialize_alsa_lib(
        get_property(config, "volume", "mix_name", "Master"),
        get_property(config, "volume", "card",     "default")
    );
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

    print_fmt("full_text", full_text_format);
    if (short_text_format)
        print_fmt("short_text", short_text_format);
}
} /* extern "C" */
