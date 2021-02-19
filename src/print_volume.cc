#include "alsa.h"
#include "printer.hpp"
#include "print_volume.h"

extern "C" {
void init_alsa(const char *mix_name, const char *card)
{
    initialize_alsa_lib(mix_name, card);
}

void print_volume()
{
    swaystatus::print("vol {}%", get_audio_volume());
}
}
