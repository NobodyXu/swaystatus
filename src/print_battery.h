#ifndef  __swaystatus_print_battery_H__
# define __swaystatus_print_battery_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_battery_monitor(const void *config);
void print_battery();

# ifdef __cplusplus
}
# endif

#endif
