#ifndef  __swaystatus_process_configuration_H__
# define __swaystatus_process_configuration_H__

# ifdef __cplusplus
extern "C" {
# endif

# include <stdbool.h>
# include <stdint.h>
# include <stddef.h>
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

/**
 * @param moduleOrder would be filled with names of the module
 *                    It is valid as long as the config is not freed.
 * @param len length of moduleOrder. If the order specified in config is greater than len,
 *            then it is silently truncated.
 * @return past-the-end iterator of moduleOrder if property order present in config,
 *         otherwise NULL.
 */
const char** get_module_order(void *config, const char* moduleOrder[], size_t len);

/**
 * @return false if the module is explicitly disabled, else true
 */
bool is_block_printer_enabled(const void *config, const char *name);

void free_config(void *config);

/**
 * The 5 getters below are used in 'modules/\*Printer.cc' only
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

const void* get_callable(const void *module_config, const char *property_name);
const void* get_click_event_handler(const void *module_config);

/**
 * @param n number of variadic args
 * @param args should be properties to be removed before converting this module_config 
 *             the a string `"property0": val, ...`, suitable for printing a json directly
 *             Eg: "\"border_top\":2,\"borer_left\":3"
 * @return If user hasn't specialized any property, then NULL
 *
 * If user hasn't specified "separator", then it would be set to true.
 */
const char* get_user_specified_property_str_impl(void *module_config, unsigned n, /* args */ ...);
const char* get_user_specified_property_str_impl2(void *module_config, unsigned n, va_list ap);

# ifdef __cplusplus
}
# endif

#endif
