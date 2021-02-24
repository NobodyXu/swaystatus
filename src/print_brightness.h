#ifndef  __swaystatus_print_brightness_H__
# define __swaystatus_print_brightness_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_brightness_detection(const char *format_str, uint32_t interval);
void print_brightness();

# ifdef __cplusplus
}
# endif

#endif
