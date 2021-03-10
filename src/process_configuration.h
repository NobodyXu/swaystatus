#ifndef  __swaystatus_process_configuration_H__
# define __swaystatus_process_configuration_H__

# ifdef __cplusplus
extern "C" {
# endif

# include <stdbool.h>
# include <stdint.h>

/**
 * load and verify the configuration.
 */
void* load_config(const char *filename);
void free_config(void *config);

/**
 * The 3 getters below are used in 'print_*.cc' only
 */

/**
 * @return heap-allocated string
 */
const char* get_property(const void *config, const char *name, const char *property,
                         const char *default_val);
/**
 * @return heap-allocated string
 */
const char* get_format(const void *config, const char *name, const char *default_val);
const char* get_short_format(const void *config, const char *name, const char *default_val);
uint32_t get_update_interval(const void *config, const char *name, uint32_t default_val);

/**
 * The 2 getters below are only used in swaystatus.c
 */

/**
 */
typedef void (*Init_t)(const void *config);
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
 */
void parse_inits_config(void *config, struct Inits *inits);

typedef void (*Printer_t)();
/**
 * All members are variable length arrays
 * The last entry will always be set to NULL.
 */
struct Blocks {
    Printer_t full_text_printers[9];
    /**
     * Each element contains JSON elements for brightness;
     * Eg: "\"border_top\":2,\"borer_left\":3"
     */
    const char *JSON_elements_strs[9];
};
/**
 * @param config after this function is invoked, all "*.format", "*.mix_name", "*.card" and all
 *               other properties that is not specified by swaybar-protocol will be removed,
 *               but it will still be a valid object after this function call
 *
 * This function will convert config to struct JSON_elements_strs and apply default value
 * to certain properties:
 *  - separator
 * if not specified by user.
 */
void parse_block_printers_config(
    void *config,
    const char * const order[9],
    struct Blocks *blocks
);

# ifdef __cplusplus
}
# endif

#endif
