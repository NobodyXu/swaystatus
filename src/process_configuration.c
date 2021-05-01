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

const char** get_module_order(void *config, const char* moduleOrder[], size_t len)
{
    if (config == NULL)
        return NULL;

    struct json_object *order;
    if (!json_object_object_get_ex(config, "order", &order))
        return NULL;

    size_t out = 0;
    size_t n = json_object_array_length(order);
    n = n > len ? len : n;
    for (size_t i = 0; i != n; i++) {
        moduleOrder[out++] = json_object_get_string(json_object_array_get_idx(order, i));
    }

    return moduleOrder + out;
}

bool is_block_printer_enabled(const void *config, const char *name)
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
    if (result == NULL) {
        if (default_val)
            return strdup_checked(default_val);
        return NULL;
    } else
        return strdup_checked(result);
}
const char* get_format(const void *module_config, const char *default_val)
{
    const char *fmt = get_property_impl(module_config, "format");
    if (fmt) {
        return escape_quotation_marks(fmt);
    } else {
        if (default_val)
            return strdup_checked(default_val);
        return NULL;
    }
}
const char* get_short_format(const void *module_config, const char *default_val)
{
    const char *fmt = get_property_impl(module_config, "short_format");
    if (fmt) {
        return escape_quotation_marks(fmt);
    } else {
        if (default_val)
            return strdup_checked(default_val);
        return NULL;
    }
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
    va_list ap;
    va_start(ap, n);
    const char *ret = get_user_specified_property_str_impl2(module_config, n, ap);
    va_end(ap);
    return ret;
}
const char* get_user_specified_property_str_impl2(void *module_config, unsigned n, va_list ap)
{
    if (!module_config)
        return NULL;

    va_list args;
    va_copy(args, ap);

    json_object_object_del(module_config, "format");
    json_object_object_del(module_config, "update_interval");
    json_object_object_del(module_config, "click_event_handler");
    for (unsigned i = 0; i != n; ++i) {
        json_object_object_del(module_config, va_arg(args, const char*));
    }

    va_end(args);

    if (json_object_object_length(module_config) == 0)
        return NULL;

    size_t json_str_len;
    const char *json_str = json_object_to_json_string_length(
        module_config,
        json2str_flag,
        &json_str_len
    );

    size_t size = json_str_len;

#define DEFAULT_PROPERTY "\"separator\":true"

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
const void* get_click_event_handler(const void *module_config)
{
    struct json_object *click_event_handler;
    if (!json_object_object_get_ex(module_config, "click_event_handler", &click_event_handler))
        return NULL;
    return click_event_handler;
}
