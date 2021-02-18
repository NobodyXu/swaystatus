#include <stddef.h>
#include <string.h>

#include <assert.h>
#include <err.h>

#include <json.h>
#include <json_object.h>
#include <json_util.h>

#include "utility.h"
#include "process_configuration.h"

static const int json2str_flag = JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOZERO;

static const char * const valid_names[] = {
    "brightness",
    "volume",
    "battery",
    "network_interface",
    "load",
    "memory_usage",
    "time",
};
static const size_t valid_name_sz = sizeof(valid_names) / sizeof(const char*);

struct Property {
    const char *name;
    json_type type;
};
static const struct Property valid_properties[] = {
    {"format", json_type_string},
    {"color", json_type_string},
    {"background", json_type_string},
    {"border", json_type_string},
    {"border_top", json_type_int},
    {"border_bottom", json_type_int},
    {"border_left", json_type_int},
    {"border_right", json_type_int},
    {"min_width", json_type_int},
    {"align", json_type_string},
    {"separator", json_type_boolean},
    {"separator_block_width", json_type_int},
};
static const size_t valid_property_sz = sizeof(valid_properties) / sizeof(struct Property);

static void verify_entry(const char *filename, const char *name, struct json_object *entry)
{
    json_object_object_foreach(entry, property, val) {
        if (strcmp(name, "volume") == 0) {
            if (strcmp(property, "mix_name") == 0 || strcmp(property, "card") == 0) {
                if (json_object_get_type(val) != json_type_string)
                    errx(1, "Invalid type of value for %s.%s in %s", name, property, filename);
                continue;
            }
        }

        size_t i = 0;
        for (; i != valid_property_sz; ++i) {
            if (strcmp(property, valid_properties[i].name) == 0) {
                if (json_object_get_type(val) != valid_properties[i].type)
                    errx(1, "Invalid type of value for %s.%s in %s", name, property, filename);
                break;
            }
        }
        if (i == valid_property_sz)
            errx(1, "Invalid property %s of %s found in %s", property, name, filename);
    }
}
static void verify_config(const char *filename, struct json_object *config)
{
    json_object_object_foreach(config, name, properties) {
        size_t i = 0;
        for (; i != valid_name_sz; ++i) {
            if (strcmp(name, valid_names[i]) == 0)
                break;
        }
        if (i == valid_name_sz)
            errx(1, "Invalid name %s found in %s", name, filename);
        if (json_object_get_type(properties) != json_type_object)
            errx(1, "Invalid value for name %s found in %s", name, filename);
        verify_entry(filename, name, properties);
    }
}
void* load_config(const char *filename)
{
    struct json_object *config = json_object_from_file(filename);
    if (!config)
        errx(1, "%s on %s failed: %s", "json_object_from_file", filename, json_util_get_last_err());
    verify_config(filename, config);

    return config;
}
void free_config(void *config)
{
    json_object_put(config);
}

const char* get_property(void *config, const char *name, const char *property,
                         const char *default_val)
{
    if (!config)
        return default_val;

    struct json_object *properties;
    if (!json_object_object_get_ex(config, name, &properties))
        return default_val;
    
    struct json_object *value;
    if (!json_object_object_get_ex(properties, property, &value))
        return default_val;

    return strdup_checked(json_object_get_string(value));
}
const char* get_format(void *config, const char *name, const char *default_val)
{
    return get_property(config, name, "format", default_val);
}

static int has_seperator(struct json_object *properties)
{
    struct json_object *separator;
    return json_object_object_get_ex(properties, "separator", &separator);
}
static const char* get_elements_str(void *config, const char *name)
{
#define DEFAULT_PROPERTY "\"separator\":true"

    struct json_object *properties;
    if (!json_object_object_get_ex(config, name, &properties))
        return DEFAULT_PROPERTY;

    json_object_object_del(properties, "format");
    if (strcmp(name, "volume") == 0) {
        json_object_object_del(properties, "mix_name");
        json_object_object_del(properties, "card");
    }

    size_t json_str_len;
    const char *json_str = json_object_to_json_string_length(
        properties,
        json2str_flag,
        &json_str_len
    );

    size_t size = json_str_len;

    const int has_sep = has_seperator(properties);
    if (!has_sep)
        size += sizeof(DEFAULT_PROPERTY) - 1;

    size = size - /* Remove '{' and '}' */ 2 + 1;
    char *ret = malloc_checked(size);
    memcpy(ret, json_str + 1, json_str_len - 2);
    if (!has_sep)
        memcpy(ret + json_str_len - 2, DEFAULT_PROPERTY, sizeof(DEFAULT_PROPERTY) - 1);
    ret[size - 1] = '\0';

    return ret;
}
void config2json_elements_strs(void *config, struct JSON_elements_strs *elements)
{
    elements->brightness = get_elements_str(config, "brightness");
    elements->volume = get_elements_str(config, "volume");
    elements->battery = get_elements_str(config, "battery");
    elements->network_interface = get_elements_str(config, "network_interface");
    elements->load = get_elements_str(config, "load");
    elements->memory_usage = get_elements_str(config, "memory_usage");
    elements->time = get_elements_str(config, "time");
}