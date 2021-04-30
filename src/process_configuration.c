#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include <assert.h>
#include <err.h>
#include <errno.h>

#include <json.h>
#include <json_object.h>
#include <json_util.h>

#include "utility.h"
#include "handle_click_events.h"
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

static size_t find_valid_name(const char *name)
{
    size_t i = 0;
    for (; i != valid_name_sz; ++i) {
        if (strcmp(name, valid_names[i]) == 0)
            break;
    }
    return i;
}
void* load_config(const char *filename)
{
    struct json_object *config = json_object_from_file(filename);
    if (!config)
        errx(1, "%s on %s failed: %s", "json_object_from_file", filename, json_util_get_last_err());

    return config;
}
void free_config(void *config)
{
    json_object_put(config);
}

void* get_module_config(void *config, const char *name)
{
    if (!config)
        return NULL;

    struct json_object *module;
    if (!json_object_object_get_ex(config, name, &module))
        return NULL;

    if (json_object_get_type(module) == json_type_boolean)
        return NULL;
    
    return module;
}

const char* get_property_impl(const void *module_config, const char *property)
{
    if (!module_config)
        return NULL;

    struct json_object *value;
    if (!json_object_object_get_ex(module_config, property, &value))
        return NULL;

    return json_object_get_string(value);
}
const char* get_property(const void *module_config, const char *property, const char *default_val)
{
    const char *result = get_property_impl(module_config, property);
    if (result == NULL)
        return default_val;
    else
        return strdup_checked(result);
}
const char* get_format(const void *module_config, const char *default_val)
{
    const char *fmt = get_property_impl(module_config, "format");
    return fmt ? escape_quotation_marks(fmt) : default_val;
}
const char* get_short_format(const void *module_config, const char *default_val)
{
    const char *fmt = get_property_impl(module_config, "short_format");
    return fmt ? escape_quotation_marks(fmt) : default_val;
}
uint32_t get_update_interval(const void *module_config, const char *name, uint32_t default_val)
{
    if (!module_config)
        return default_val;

    struct json_object *value;
    if (!json_object_object_get_ex(module_config, "update_interval", &value))
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

static int has_seperator(const struct json_object *properties)
{
    struct json_object *separator;
    return json_object_object_get_ex(properties, "separator", &separator);
}
const char* get_user_specified_property_str_impl(void *module_config, unsigned n, ...)
{
#define DEFAULT_PROPERTY "\"separator\":true"
    if (!module_config)
        return DEFAULT_PROPERTY;

    va_list ap;
    va_start(ap, n);

    json_object_object_del(module_config, "format");
    json_object_object_del(module_config, "update_interval");
    for (unsigned i = 0; i != n; ++i) {
        json_object_object_del(module_config, va_arg(ap, const char*));
    }

    va_end(ap);

    if (json_object_object_length(module_config) == 0)
        return DEFAULT_PROPERTY;

    size_t json_str_len;
    const char *json_str = json_object_to_json_string_length(
        module_config,
        json2str_flag,
        &json_str_len
    );

    size_t size = json_str_len;

    const int has_sep = has_seperator(module_config);
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

int init_click_event_handlers(void *config, const char *names[9], int force_enabled)
{
    init_click_events_handling();

    if (!config)
        return 1;

    for (size_t i = 0; names[i]; ++i) {
        const char *name = names[i];

        struct json_object *block;
        if (!json_object_object_get_ex(config, name, &block))
            continue;

        struct json_object *click_event_handler;
        if (!json_object_object_get_ex(block, "click_event_handler", &click_event_handler))
            continue;

        add_click_event_handler(name, click_event_handler);
    }

    return 1;
}

void get_block_printers(const char * const order[9], struct Blocks *blocks)
{
    size_t out = 0;
    for (size_t i = 0; order[i]; ++i) {
        size_t name_index = find_valid_name(order[i]);
        blocks->full_text_printers[out] = default_full_text_printers[name_index];
        ++out;
    }
    blocks->full_text_printers[out] = NULL;
}
