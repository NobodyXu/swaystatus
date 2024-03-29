#include <cstring>

#include <json_object.h>

#include "Callable.hpp"

namespace swaystatus {
static const char *get_str_from_config(
    const char *name,
    const json_object *callable_config,
    const char *attr
)
{
    struct json_object *val;
    if (!json_object_object_get_ex(callable_config, attr, &val))
        errx(1, "Attr %s.click_event_handler/callback.%s %s", name, attr, "is missing");

    if (json_object_get_type(val) != json_type_string)
        errx(1, "Attr %s.click_event_handler/callback.%s %s", name, attr, "contains invalid value");

    return json_object_get_string(val);
}

Callable_base::Callable_base(const char *name, const void *callable_config_arg)
{
    auto *callable_config = static_cast<const json_object*>(callable_config_arg);
    const char *type = get_str_from_config(name, callable_config, "type");

    auto get_str = [&](const char *attr)
    {
        return get_str_from_config(name, callable_config, attr);
    };

    auto *module_name = get_str("module_name");
    auto *function_name = get_str("function_name");

    if (std::strcmp(type, "python") == 0) {
#ifdef USE_PYTHON
        python::MainInterpreter::load_libpython3();
        auto scope = python::MainInterpreter::get().acquire();

        auto module = [&]{
            struct json_object *code;
            if (json_object_object_get_ex(callable_config, "code", &code)) {
                python::Compiled compiled{module_name, get_str("code")};
                return python::Module{module_name, compiled};
            } else
                return python::Module{module_name};
        }();

        v.emplace<python::Callable_base>(module.getattr(function_name));
#else
        errx(1, "Click events specified using python callback, but feature python is not "
                "supported.");
#endif
    } else if (std::strcmp(type, "dylib") == 0) {
        v.emplace<void*>(dload_symbol(module_name, function_name));
    } else
        errx(1, "Attr %s.click_event_handler.%s %s", name, "type", "contains invalid value");
}
} /* namespace swaystatus */
