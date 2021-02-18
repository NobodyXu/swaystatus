#ifndef  __swaystatus_process_configuration_H__
# define __swaystatus_process_configuration_H__

/**
 * load and verify the configuration.
 */
void* load_config(const char *filename);
void free_config(void *config);

const char* get_property(void *config, const char *name, const char *property,
                         const char *default_val);
const char* get_format(void *config, const char *name, const char *default_val);

struct JSON_elements_strs {
    /**
     * Contains JSON elements for brightness;
     * Eg: "\"border_top\":2,\"borer_left\":3"
     */
    const char *brightness;
    const char *volume;
    const char *battery;
    const char *network_interface;
    const char *load;
    const char *memory_usage;
    const char *time;
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
void config2json_elements_strs(void *config, struct JSON_elements_strs *elements);

#endif
