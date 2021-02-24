#ifndef  __swaystatus_print_battery_H__
# define __swaystatus_print_battery_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_battery_monitor(const char *format_str, uint32_t interval);
void print_battery();

# ifdef __cplusplus
}
# endif

#endif
