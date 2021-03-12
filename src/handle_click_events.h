#ifndef  __swaystatus_handle_click_events_H__
# define __swaystatus_handle_click_events_H__

# include <stddef.h>

# ifdef __cplusplus
extern "C" {
# endif

enum ClickEventHandlerType {
    handle_type_python,
};

struct ClickEventHandlerConfig {
    /**
     * should be a pointer to .rodata
     */
    const char *name;

    enum ClickEventHandlerType type;
    union {
        struct {
            const char *module_name;
            /**
             * If code == NULL, then the module is loaded from filesystem;
             * Else, the code is treated as module and module_name is treated as
             * its name.
             */
            const char *code;
            const char *function_name;
        } python;
    };
};

/**
 * @param out if equals to 0, then click events will be discarded.
 */
void init_click_events_handling(const struct ClickEventHandlerConfig *handlers, size_t cnt);

# ifdef __cplusplus
}
# endif

#endif
