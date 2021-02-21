#ifndef  __swaystatus_print_network_interfaces_H__
# define __swaystatus_print_network_interfaces_H__

# ifdef __cplusplus
extern "C" {
# endif

void init_network_interfaces_scanning(const char *format_str);
void print_network_interfaces();

# ifdef __cplusplus
}
# endif

#endif
