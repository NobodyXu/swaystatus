#ifndef  __swaystatus_handle_click_events_H__
# define __swaystatus_handle_click_events_H__

# include <stddef.h>
# include <stdint.h>

# ifdef __cplusplus
extern "C" {

enum class ClickHandlerRequest: uint8_t {
    none = 0,
    update = 1,
    reload = 2,
};

inline uint8_t operator & (const ClickHandlerRequest &x, const ClickHandlerRequest &y) noexcept
{
    return static_cast<uint8_t>(x) & static_cast<uint8_t>(y);
}

inline uint8_t operator |= (ClickHandlerRequest &x, const ClickHandlerRequest &y) noexcept
{
    auto ret = static_cast<uint8_t>(static_cast<uint8_t>(x) | static_cast<uint8_t>(y));
    x = ClickHandlerRequest{ret};
    return ret;
}

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
