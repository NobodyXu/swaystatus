#ifndef  __swaystatus_print_volume_H__
# define __swaystatus_print_volume_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_volume_monitor(
    const char *format_str, uint32_t interval,
    const char *mix_name, const char *card
);
void print_volume();

# ifdef __cplusplus
}
# endif

#endif
