/**
 * functions relating to alsa are put here since alsa/asoundlib.h doesn't work well with C++
 */
#ifndef  __swaystatus_alsa_H__
# define __swaystatus_alsa_H__

# ifdef __cplusplus
extern "C" {
# endif

void initialize_alsa_lib(const char *mix_name, const char *card);

void update_volume();
long get_audio_volume();

# ifdef __cplusplus
}
# endif

#endif
