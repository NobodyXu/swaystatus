#ifndef  __swaystatus_handle_click_events_H__
# define __swaystatus_handle_click_events_H__

# include <stddef.h>
# include <stdint.h>

# ifdef __cplusplus
extern "C" {

enum class ClickHandlerRequest: uint8_t {
    none = 0,
    update = 1 << 0,
    reload = 1 << 1,
};

# endif

void init_click_events_handling();

/**
 * @param click_event_handler_config if equals to NULL, return without doing anything.
 * @return NULL if click_event_handler_config is NULL, otherwise it will be the events 
 *              requested by the callback (a bitwise or of all return value)
 * 
 * Be sure to set the *(ret ptr) to 0 after you processed all events in it.
 */
uint8_t* add_click_event_handler(const char *name, const void *click_event_handler_config);

# ifdef __cplusplus
}
# endif

#endif
