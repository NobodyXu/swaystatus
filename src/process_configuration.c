#include <stddef.h>
#include <string.h>

#include <assert.h>
#include <err.h>
#include <errno.h>

#include <json.h>
#include <json_object.h>
#include <json_util.h>

#include "utility.h"
#include "print_battery.h"
#include "print_time.h"
#include "print_volume.h"
#include "print_network_interfaces.h"
#include "print_brightness.h"
#include "print_memory_usage.h"
#include "print_load.h"
#include "print_sensors.h"
#include "process_configuration.h"

static const int json2str_flag = JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOZERO;

/**
 * This array also defines the default order of blocks.
 */
static const char * const valid_names[] = {
    "brightness",
    "volume",
    "battery",
    "network_interface",
    "load",
    "memory_usage",
    "sensors",
    "time",
};
static const size_t valid_name_sz = sizeof(valid_names) / sizeof(const char*);

static const Init_t default_inits[] = {
    init_brightness_detection,
    init_volume_monitor,
    init_battery_monitor,
    init_network_interfaces_scanning,
    init_load,
    init_memory_usage_collection,
    init_sensors,
    init_time,
};
_Static_assert(sizeof(default_inits) < (offsetof(struct Inits, order)), "");
_Static_assert(
    sizeof(default_inits) / sizeof(Init_t) == sizeof(valid_names) / sizeof(const char*),
    ""
);

static const Printer_t default_full_text_printers[] = {
    print_brightness,
    print_volume,
    print_battery,
    print_network_interfaces,
    print_load,
    print_memory_usage,
    print_sensors,
    print_time,
};
_Static_assert(
    sizeof(default_full_text_printers) / sizeof(Printer_t) == 
        sizeof(valid_names) / sizeof(const char*),
    ""
);

struct Property {
    const char *name;
    json_type type;
};
static const struct Property valid_properties[] = {
    {"format", json_type_string},
    {"short_format", json_type_string},
    {"update_interval", json_type_int},
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
    {"markup", json_type_string},
};
static const size_t valid_property_sz = sizeof(valid_properties) / sizeof(struct Property);

static size_t find_valid_name(const char *name)
{
    size_t i = 0;
    for (; i != valid_name_sz; ++i) {
        if (strcmp(name, valid_names[i]) == 0)
            break;
    }
    return i;
}
static bool is_valid_name(const char *name)
{
    if (find_valid_name(name) != valid_name_sz)
        return true;
    else
        return false;
}
static void verify_order(const char *filename, struct json_object *entry)
{
    if (json_object_get_type(entry) != json_type_array)
        errx(1, "Invalid type of value for %s.%s in %s", "", "order", filename);

    const size_t n = json_object_array_length(entry);
    if (n > valid_name_sz)
        errx(1, "Invalid order in %s: Cannot have more elements than %zu", filename, valid_name_sz);

    for (size_t i = 0; i != n; i++) {
        struct json_object *object = json_object_array_get_idx(entry, i);
        if (json_object_get_type(object) != json_type_string)
            errx(1, "Invalid type of value for %s[%zu] in %s", "order", i, filename);

        const char *name = json_object_get_string(object);
        if (!is_valid_name(name))
            errx(1, "Invalid name %s found in %s[%zu], %s", name, "order", i, filename);
    }
}
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

        /* Ignore JSON comments */
        if (property[0] == '_') {
            json_object_object_del(entry, property);
            continue;
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
        /* Ignore JSON comments */
        if (name[0] == '_')
            continue;
        if (strcmp(name, "order") == 0) {
            verify_order(filename, properties);
            continue;
        }

        if (!is_valid_name(name))
            errx(1, "Invalid name %s found in %s", name, filename);

        json_type properties_type = json_object_get_type(properties);
        if (properties_type != json_type_object && properties_type != json_type_boolean)
            errx(1, "Invalid value for name %s found in %s", name, filename);
        if (properties_type == json_type_object)
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

const char* get_property_impl(const void *config, const char *name, const char *property)
{
    if (!config)
        return NULL;

    struct json_object *properties;
    if (!json_object_object_get_ex(config, name, &properties))
        return NULL;

    if (json_object_get_type(properties) == json_type_boolean)
        return NULL;
    
    struct json_object *value;
    if (!json_object_object_get_ex(properties, property, &value))
        return NULL;

    return json_object_get_string(value);
}
const char* get_property(const void *config, const char *name, const char *property,
                         const char *default_val)
{
    const char *result = get_property_impl(config, name, property);
    if (result == NULL)
        return default_val;
    else
        return strdup_checked(result);
}
const char* get_format(const void *config, const char *name, const char *default_val)
{
    const char *fmt = get_property_impl(config, name, "format");
    return fmt ? escape_quotation_marks(fmt) : default_val;
}
const char* get_short_format(const void *config, const char *name, const char *default_val)
{
    const char *fmt = get_property_impl(config, name, "short_format");
    return fmt ? escape_quotation_marks(fmt) : default_val;
}
uint32_t get_update_interval(const void *config, const char *name, uint32_t default_val)
{
    if (!config)
        return default_val;

    struct json_object *properties;
    if (!json_object_object_get_ex(config, name, &properties))
        return default_val;

    if (json_object_get_type(properties) == json_type_boolean)
        return default_val;
    
    struct json_object *value;
    if (!json_object_object_get_ex(properties, "update_interval", &value))
        return default_val;

    errno = 0;
    int64_t interval = json_object_get_int64(value);
    if (errno != 0)
        err(1, "%s on %s.%s%s", "json_object_get_uint64", name, "update_interval", " failed");
    if (interval > UINT32_MAX)
        errx(1, "%s on %s.%s%s", "Value too large", name, "update_interval", "");
    if (interval < 0)
        errx(1, "%s on %s.%s%s", "Negative number is not accepted", name, "update_interval", "");

    return interval;
}

static bool is_block_printer_enabled(const void *config, const char *name)
{
    if (config == NULL)
        return true;

    struct json_object *val;
    if (!json_object_object_get_ex(config, name, &val))
        return true;
    if (json_object_get_type(val) == json_type_object)
        return true;

    return json_object_get_boolean(val);
}

static void get_default_order(const void *config, struct Inits *inits)
{
    size_t out = 0;
    for (size_t i = 0; i != valid_name_sz; ++i) {
        if (is_block_printer_enabled(config, valid_names[i])) {
            inits->inits[out] = default_inits[i];
            inits->order[out++] = valid_names[i];
        }
    }
    inits->inits[out] = NULL;
    inits->order[out] = NULL;
}
void parse_inits_config(void *config, struct Inits *inits)
{
    if (config == NULL)
        return get_default_order(config, inits);

    struct json_object *order;
    if (!json_object_object_get_ex(config, "order", &order))
        return get_default_order(config, inits);

    size_t out = 0;
    const size_t n = json_object_array_length(order);
    for (size_t i = 0; i != n; i++) {
        const char *name = json_object_get_string(json_object_array_get_idx(order, i));

        if (is_block_printer_enabled(config, name)) {
            size_t name_index = find_valid_name(name);
            inits->inits[out] = default_inits[name_index];
            inits->order[out++] = valid_names[name_index];
        }
    }
    inits->inits[out] = NULL;
    inits->order[out] = NULL;

    json_object_object_del(config, "order");
}

static int has_seperator(const struct json_object *properties)
{
    struct json_object *separator;
    return json_object_object_get_ex(properties, "separator", &separator);
}
static const char* get_elements_str(const void *config, const char *name)
{
#define DEFAULT_PROPERTY "\"separator\":true"

    struct json_object *properties;
    if (!json_object_object_get_ex(config, name, &properties))
        return DEFAULT_PROPERTY;

    if (json_object_get_type(properties) == json_type_boolean)
        return DEFAULT_PROPERTY;

    json_object_object_del(properties, "format");
    json_object_object_del(properties, "update_interval");
    if (strcmp(name, "volume") == 0) {
        json_object_object_del(properties, "mix_name");
        json_object_object_del(properties, "card");
    }

    if (json_object_object_length(properties) == 0)
        return DEFAULT_PROPERTY;

    size_t json_str_len;
    const char *json_str = json_object_to_json_string_length(
        properties,
        json2str_flag,
        &json_str_len
    );

    size_t size = json_str_len;

    const int has_sep = has_seperator(properties);
    if (!has_sep)
        size += /* For the comma */ 1 + sizeof(DEFAULT_PROPERTY) - 1;

    size = size - /* Remove '{' and '}' */ 2 + 1;
    char *ret = malloc_checked(size);
    memcpy(ret, json_str + 1, json_str_len - 2);
    if (!has_sep) {
        char *dest = ret + json_str_len - 2;
        *dest++ = ',';
        memcpy(dest, DEFAULT_PROPERTY, sizeof(DEFAULT_PROPERTY) - 1);
    }
    ret[size - 1] = '\0';

    return ret;
}

void parse_block_printers_config(
    void *config,
    const char * const order[9],
    struct Blocks *blocks
)
{
    size_t out = 0;
    for (size_t i = 0; order[i]; ++i) {
        size_t name_index = find_valid_name(order[i]);
        blocks->full_text_printers[out] = default_full_text_printers[name_index];
        blocks->JSON_elements_strs[out++] = get_elements_str(config, order[i]);
    }
    blocks->full_text_printers[out] = NULL;
    blocks->JSON_elements_strs[out] = NULL;
}
