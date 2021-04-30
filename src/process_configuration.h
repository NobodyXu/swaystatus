#ifndef  __swaystatus_process_configuration_H__
# define __swaystatus_process_configuration_H__

# ifdef __cplusplus
extern "C" {
# endif

# include <stdbool.h>
# include <stdint.h>
# include <stdarg.h>

/**
 * load and verify the configuration.
 */
void* load_config(const char *filename);

/**
 * @param config can be NULL
 * @param name must not be NULL, can be battery, network_interface, etc.
 * @return If config == NULL, then return NULL. If the module config does not exist, return NULL.
 *         If the module config is a boolean, return NULL
 *         Otherwise a non-NULL value.
 */
void* get_module_config(void *config, const char *name);

void free_config(void *config);

/**
 * The 5 getters below are used in 'print_*.cc' only
 */

/**
 * @return heap-allocated string
 */
const char* get_property(const void *module_config, const char *property, const char *default_val);
/**
 * @return heap-allocated string
 */
const char* get_format(const void *module_config, const char *default_val);
const char* get_short_format(const void *module_config, const char *default_val);
/**
 * @param module_name used only for printing err msg
 */
uint32_t get_update_interval(const void *module_config, const char *module_name, uint32_t default_val);

const void* get_click_event_handler(const void *module_config);

/**
 * @param n number of variadic args
 * @param args should be properties to be removed before converting this module_config 
 *             the a string `"property0": val, ...`, suitable for printing a json directly
 *             Eg: "\"border_top\":2,\"borer_left\":3"
 *
 * This function will convert config to struct JSON_elements_strs and apply default value
 * to certain properties:
 *  - separator
 * if not specified by user.
 */
const char* get_user_specified_property_str_impl(void *module_config, unsigned n, /* args */ ...);
const char* get_user_specified_property_str_impl2(void *module_config, unsigned n, va_list ap);

/**
 * The 3 functions below are only used in swaystatus.c
 */

/**
 */
typedef void (*Init_t)(void *config);
/**
 * All members are variable length arrays
 * The last entry will always be set to NULL.
 */
struct Inits {
    Init_t inits[9];
    /**
     * Elements in here points to .rodata section.
     */
    const char *order[9];
};
/**
 * @param config after this function is invoked, element 'order' is removed.
 *
 * This function also calls init_click_events_handling().
 */
void parse_inits_config(void *config, struct Inits *inits);

typedef void (*Printer_t)();
/**
 * All members are variable length arrays
 * The last entry will always be set to NULL.
 */
struct Blocks {
    Printer_t full_text_printers[9];
};
/**
 */
void get_block_printers(const char * const order[9], struct Blocks *blocks);

# ifdef __cplusplus
}

#include <utility>

namespace swaystatus {
template <class ...Args>
const char* get_user_specified_property_str(void *module_config, Args &&...args)
{
    return ::get_user_specified_property_str_impl(module_config, sizeof...(args), std::forward<Args>(args)...);
}
} /* namespace swaystatus */
# endif

#endif
