#ifndef  __swaystatus_print_load_H__
# define __swaystatus_print_load_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_load(const char *format_str, uint32_t interval);
void print_load();

# ifdef __cplusplus
}
# endif

#endif
