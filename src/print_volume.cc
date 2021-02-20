#include "alsa.h"
#include "printer.hpp"
#include "print_volume.h"

extern "C" {
static const char *format;

void init_volume_monitor(const char *format_str, const char *mix_name, const char *card)
{
    format = format_str;
    initialize_alsa_lib(mix_name, card);
}

void print_volume()
{
    swaystatus::print(format, fmt::arg("volume", get_audio_volume()));
}
}
