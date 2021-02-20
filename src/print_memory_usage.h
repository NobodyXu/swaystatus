#ifndef  __swaystatus_print_memory_usage_H__
# define __swaystatus_print_memory_usage_H__

# ifdef __cplusplus
extern "C" {
# endif

void init_memory_usage_collection(const char *format_str);
void print_memory_usage();

# ifdef __cplusplus
}
# endif

#endif
