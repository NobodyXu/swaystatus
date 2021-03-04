#ifndef  __swaystatus_sensors_H__
# define __swaystatus_sensors_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_sensors(const char *format, uint32_t interval);
void print_sensors();

# ifdef __cplusplus
}
# endif

#endif
