#include "alsa.h"
#include "printer.hpp"
#include "print_volume.h"

static uint32_t cycle_cnt;
static uint32_t interval;

extern "C" {
static const char *format;

void init_volume_monitor(
    const char *format_str, uint32_t interval_arg,
    const char *mix_name, const char *card
)
{
    format = format_str;
    interval = interval_arg;

    initialize_alsa_lib(mix_name, card);
}

void print_volume()
{
    if (cycle_cnt++ == interval) {
        cycle_cnt = 0;
        update_volume();
    }

    swaystatus::print(format, fmt::arg("volume", get_audio_volume()));
}
}
