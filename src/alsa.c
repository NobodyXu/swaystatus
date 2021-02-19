/**
 * The file is adatped from:
 * https://stackoverflow.com/questions/7657624/get-master-sound-volume-in-c-in-linux, which uses
 * WTFPL license.
 */

/**
 * Fix redefinition of timespec and timeval in alsa/global.h.
 *
 * Learnt from:
 * https://stackoverflow.com/questions/32672333/including-alsa-asoundlib-h-and-sys-time-h-results-in-multiple-definition-co
 */
#define _POSIX_C_SOURCE 200809L

#include <alsa/asoundlib.h>

#include <err.h>
#include <alloca.h>

#include "alsa.h"

static snd_mixer_t *handle;
static snd_mixer_elem_t *elem;
static long volume;

static long calculate_audio_volume();

void initialize_alsa_lib(const char *mix_name, const char *card)
{
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);

    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, mix_name);

    if (snd_mixer_open(&handle, 0) < 0)
        errx(1, "%s failed", "snd_mixer_open");

    if (snd_mixer_attach(handle, card) < 0)
        errx(1, "%s failed", "snd_mixer_attach");

    if (snd_mixer_selem_register(handle, NULL, NULL) < 0)
        errx(1, "%s failed", "snd_mixer_selem_register");

    if (snd_mixer_load(handle) < 0)
        errx(1, "%s failed", "snd_mixer_load");

    elem = snd_mixer_find_selem(handle, sid);
    if (!elem)
        errx(1, "%s failed", "snd_mixer_find_selem");

    volume = calculate_audio_volume();
}

static long calculate_audio_volume()
{
    long minv, maxv;
    snd_mixer_selem_get_playback_volume_range(elem, &minv, &maxv);

    long vol;
    if (snd_mixer_selem_get_playback_volume(elem, 0, &vol) < 0)
        errx(1, "%s failed", "snd_mixer_selem_get_playback_volume");

    /* make the vol bound to range [0, 100] */
    vol -= minv;
    maxv -= minv;
    return 100 * vol / maxv;
}
long get_audio_volume()
{
    if (snd_mixer_wait(handle, 0) == 0) {
        if (snd_mixer_handle_events(handle) < 0)
            errx(1, "%s failed", "snd_mixer_handle_events");

        volume = calculate_audio_volume();
    }
    return volume;
}
