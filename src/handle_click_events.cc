#include <cstddef>
#include <cstdint>
#include <cstring>

#include <err.h>

#include <json_tokener.h>

#include <algorithm>
#include <variant>
#include <tuple>

#include "poller.h"
#include "utility.h"
#include "python3.hpp"
#include "handle_click_events.h"

#ifdef USE_PYTHON
using swaystatus::python::Interpreter;
using swaystatus::python::MainInterpreter;
using swaystatus::python::Compiled;
using swaystatus::python::Module;
using swaystatus::python::Int;
using swaystatus::python::Callable;
#endif

extern "C" {
struct Pos {
    std::uint64_t x;
    std::uint64_t y;
};
typedef struct Pos ClickPos;
typedef struct Pos BlockSize;
} /* extern "C" */
std::tuple<std::uint64_t, std::uint64_t> to_unpackable(const struct Pos &pos)
{
    return {pos.x, pos.y};
}

#ifdef USE_PYTHON
using py_callback = Callable</* Ret type */ Int,
    /* Args types */
    const char*,
    const ClickPos &,
    std::uint64_t,
    std::uint64_t,
    const ClickPos &,
    const BlockSize &
>;
#endif

struct Callback {
    const char *name;
    std::variant<
        /* instance, click_pos, button, event, relative_click_pos, BlockSize */
        std::monostate
#ifdef USE_PYTHON
        , py_callback
#endif
    > v;

    auto operator () (
        const char *instance,
        const ClickPos &pos,
        std::uint64_t button,
        std::uint64_t event,
        const ClickPos &relative_pos,
        const BlockSize &size
    ) -> std::uint8_t
    {
        switch (v.index()) {
            case std::variant_npos:
            case 0:
                errx(1, "Internal bug: Visiting uninitialized callback in %s", __FILE__);

            default:
                break;
        }

#ifdef USE_PYTHON
        auto scope = std::holds_alternative<py_callback>(v) ? 
            MainInterpreter::get().acquire() :
            Interpreter::GIL_scoped(nullptr);
#endif

        return std::visit([&](auto &&f)
        {
            if constexpr(std::is_same_v<swaystatus::rm_cvref_t<decltype(f)>, std::monostate>)
                return std::uint8_t{0};
            else
                return static_cast<std::uint8_t>(
                    f(instance, pos, button, event, relative_pos, size)
                );
        }, v);
    }
};

static struct json_tokener *parser;
static Callback callbacks[8];
static std::size_t callback_cnt;

static void click_events_handler(int fd, enum Event events, void *data);

extern "C" {
void init_click_events_handling(const struct ClickEventHandlerConfig *handlers, std::size_t cnt)
{
    callback_cnt = cnt;

    for (size_t i = 0; i != cnt; ++i) {
        callbacks[i].name = handlers[i].name;

        switch (handlers[i].type) {
            case handle_type_python:
#ifdef USE_PYTHON
                MainInterpreter::load_libpython3();

                {
                    auto scope = MainInterpreter::get().acquire();

                    const char *module_name = handlers[i].python.module_name;
                    const char *code        = handlers[i].python.code;
                    const char *function_name = handlers[i].python.function_name;

                    auto module = [&]{
                        if (handlers[i].python.code) {
                            Compiled compiled{module_name, code};
                            return Module{module_name, compiled};
                        } else
                            return Module{module_name};
                    }();
                    callbacks[i].v.emplace<py_callback>(module.getattr(function_name));
                }
#else
                errx(1, "Click events specified using python callback, but feature python is not "
                        "support.");
#endif
        }
    }

    set_fd_non_blocking(0);

    if (cnt != 0) {
        parser = json_tokener_new();
        if (parser == NULL)
            errx(1, "%s failed", "json_tokener_new");

        json_tokener_set_flags(parser, JSON_TOKENER_ALLOW_TRAILING_CHARS);
    }

    request_polling(0, read_ready, click_events_handler, NULL);
}
} /* extern "C" */

static void click_event_handler(const struct json_object *event);
static void click_events_handler(int fd, enum Event events, void *data)
{
    if (events == invalid_fd)
        errx(1, "fd %d %s", fd, "is invalid");
    if (events == hup)
        /* swaybar is being reloaded */
        return;
    if (events == error)
        errx(1, "fd %d %s", fd, "errored");

    char buffer[4096];
    /**
     * json_tokener_parse_ex cannot accept buffer longer than INT32_MAX
     */
    _Static_assert(sizeof(buffer) <= INT32_MAX, "");

    static bool first_read = true;

    ssize_t len = readall(0, buffer, sizeof(buffer));
    if (len < 0)
        err(1, "%s on %s failed", "read", "stdin");
    if (len == 0)
        return;
    buffer[len] = '\0';
    if (first_read) {
        /**
         * The input is an infinite JSON array, so has to ignore the first
         * character '[', otherwise json_tokener_parse_ex would never
         * return any object.
         */
        buffer[0] = ' ';
        first_read = false;
    }

    if (callback_cnt == 0)
        return;

    std::size_t i = 0;
    do {
        /**
         * Ignore ',' and space from the infinite JSON array
         */
        while (buffer[i] == ',' || buffer[i] == '\n' || buffer[i] == ' ' || buffer[i] == '\t')
            ++i;

        struct json_object *event = json_tokener_parse_ex(parser, buffer + i, len - i);
        enum json_tokener_error jerr = json_tokener_get_error(parser);
        size_t bytes_processed = json_tokener_get_parse_end(parser);

        if (event != nullptr) {
            click_event_handler(event);
            json_object_put(event);
        } else if (jerr != json_tokener_continue) {
            errx(
                1,
                "%s on %s failed: %s",
                "json_tokener_parse_ex",
                "content read from stdin",
                json_tokener_error_desc(jerr)
            );
        }

        i += bytes_processed;
    } while (i != len);
}
static void click_event_handler(const struct json_object *event)
{
    if (json_object_get_type(event) != json_type_object)
        errx(1, "%s on %s failed", "Assumption", "click event read from stdin");

    auto get_str = [&](const char *key)
    {
        return json_object_get_string(json_object_object_get(event, key));
    };
    auto get_int = [&](const char *key)
    {
        return json_object_get_uint64(json_object_object_get(event, key));
    };

    const char *name = get_str("name");
    auto it = std::find_if(callbacks, callbacks + callback_cnt, [&](const auto &val)
    {
        return std::strcmp(val.name, name) == 0;
    });
    if (it == callbacks + callback_cnt)
        return;

    auto &callback = *it;
    callback(
        get_str("instance"),
        ClickPos{get_int("x"), get_int("y")},
        get_int("button"),
        get_int("event"),
        ClickPos{get_int("relative_x"), get_int("relative_y")},
        BlockSize{get_int("width"), get_int("height")}
    );
}
