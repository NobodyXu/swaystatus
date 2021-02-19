#ifndef  __swaystatus_print_volume_H__
# define __swaystatus_print_volume_H__

# ifdef __cplusplus
extern "C" {
# endif

void init_alsa(const char *mix_name, const char *card);
void print_volume();

# ifdef __cplusplus
}
# endif

#endif
