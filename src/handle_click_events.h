#ifndef  __swaystatus_handle_click_events_H__
# define __swaystatus_handle_click_events_H__

# include <stddef.h>

# ifdef __cplusplus
extern "C" {
# endif

void init_click_events_handling();

void add_click_event_handler(const char *name, const void *click_event_handler_config);

# ifdef __cplusplus
}
# endif

#endif
