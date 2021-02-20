#ifndef  __swaystatus_print_battery_H__
# define __swaystatus_print_battery_H__

# ifdef __cplusplus
extern "C" {
# endif

void init_upclient(const char *format_str);
void print_battery();

# ifdef __cplusplus
}
# endif

#endif
