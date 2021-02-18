#ifndef  __swaystatus_process_configuration_H__
# define __swaystatus_process_configuration_H__

/**
 * load and verify the configuration.
 */
void* load_config(const char *filename);

const char* get_format(void *config, const char *name, const char *default_val);

#endif
