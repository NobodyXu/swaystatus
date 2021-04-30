#ifndef  __swaystatus_print_memory_usage_H__
# define __swaystatus_print_memory_usage_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_memory_usage_collection(void *config);
void print_memory_usage();

# ifdef __cplusplus
}
# endif

#endif
