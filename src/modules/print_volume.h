#ifndef  __swaystatus_print_volume_H__
# define __swaystatus_print_volume_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_volume_monitor(void *config);
void print_volume();

# ifdef __cplusplus
}
# endif

#endif
