#include "alsa.h"
#include "process_configuration.h"
#include "printer.hpp"
#include "print_volume.h"

static uint32_t cycle_cnt;
static uint32_t interval;

extern "C" {
static const char *format;

void init_volume_monitor(const void *config)
{
    format   = get_format         (config, "volume", "vol {volume}%");
    interval = get_update_interval(config, "volume", 1);

    initialize_alsa_lib(
        get_property(config, "volume", "mix_name", "Master"),
        get_property(config, "volume", "card",     "default")
    );
}

void print_volume()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;
        update_volume();
    }

    swaystatus::print(format, fmt::arg("volume", get_audio_volume()));
}
}
