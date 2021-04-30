#ifndef  __swaystatus_print_network_interfaces_H__
# define __swaystatus_print_network_interfaces_H__

# include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_network_interfaces_scanning(void *config);
void print_network_interfaces();

# ifdef __cplusplus
}
# endif

#endif
