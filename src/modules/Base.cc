#include <err.h>

#include <cstdarg>
#include <cstring>

#include <algorithm>

#include "../error_handling.hpp"
#include "../process_configuration.h"
#include "../handle_click_events.h"
#include "../formatting/printer.hpp"

#include "Base.hpp"

#include "BacklightPrinter.hpp"
#include "BatteryPrinter.hpp"
#include "LoadPrinter.hpp"
#include "MemoryUsagePrinter.hpp"
#include "NetworkInterfacesPrinter.hpp"
#include "TemperaturePrinter.hpp"
#include "TimePrinter.hpp"
#include "VolumePrinter.hpp"
#include "CustomPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
Base::Base(
    void *config, std::string_view module_name_arg,
    std::uint32_t default_interval,
    const char *default_full_format, const char *default_short_format,
    unsigned n, ...
):
    module_name{module_name_arg},
    full_text_format{get_format(config, default_full_format)},
    short_text_format{get_short_format(config, default_short_format)},
    interval{get_update_interval(config, module_name_arg.data(), default_interval)},
    requested_events{add_click_event_handler(module_name.data(), get_click_event_handler(config))}
{
    this->cycle_cnt = interval - 1;

    std::va_list ap;
    va_start(ap, n);
    user_specified_properties_str.reset(get_user_specified_property_str_impl2(config, n, ap));
    if (user_specified_properties_str)
        user_specified_properties_str_len = std::strlen(user_specified_properties_str.get());
    va_end(ap);
}

void Base::update_and_print()
{
    if (requested_events) {
        const ClickHandlerRequest requests{*requested_events};
        *requested_events = static_cast<std::uint8_t>(ClickHandlerRequest::none);

        if (requests & ClickHandlerRequest::reload) {
            reload();
            update();
        }
        if (requests & ClickHandlerRequest::update)
            update();
    }

    if (++cycle_cnt == interval) {
        cycle_cnt = 0;
        update();
    }

    print_literal_str("{\"name\":\"");
    print_str2(module_name);
    print_literal_str("\",\"instance\":\"0\",");

    print_fmt("full_text"sv, full_text_format.get());
    if (short_text_format)
        print_fmt("short_text"sv, short_text_format.get());

    if (user_specified_properties_str)
        print_str2(user_specified_properties_str.get(), user_specified_properties_str_len);
    else
        print_literal_str("\"separator\":true");

    print_literal_str("},");
}
void Base::print_fmt(std::string_view name, const char *format)
{
    print_literal_str("\"");
    print_str2(name);
    print_literal_str("\":\"");

    TRY {
        fmt_set_calling_module(module_name.data());
        do_print(format);
        fmt_set_calling_module(nullptr);
    } CATCH (const std::exception &e) {
        errx(1, "Failed to print %s format in %s: %s",
                name.data(), module_name.data(), e.what());
    };

    print_literal_str("\",");
}

static constexpr const char * const default_order[] = {
    "brightness",
    "battery",
    "load",
    "memory_usage",
    "network_interface",
    "sensors",
    "time",
    "volume",
    "custom",
};
static constexpr auto default_order_len = sizeof(default_order) / sizeof(const char*);
static_assert(CALLBACK_CNT >= default_order_len);

static constexpr const std::size_t default_index[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
};
static constexpr auto default_index_len = sizeof(default_index) / sizeof(std::size_t);
static_assert(default_order_len >= default_index_len);

using Factory = std::unique_ptr<Base> (*)(void *config);
static constexpr const Factory factories[] = {
    makeBacklightPrinter,
    makeBatteryPrinter,
    makeLoadPrinter,
    makeMemoryUsagePrinter,
    makeNetworkInterfacesPrinter,
    makeTemperaturePrinter,
    makeTimePrinter,
    makeVolumePrinter,
    makeCustomPrinter,
};
static_assert(default_order_len == sizeof(factories) / sizeof(Factory));

static auto makeModules(void *config, const std::size_t indexes[], std::size_t len)
    -> std::vector<std::unique_ptr<Base>>
{
    std::vector<std::unique_ptr<Base>> modules;

    for (std::size_t i = 0; i != len; ++i) {
        auto index = indexes[i];

        if (!is_block_printer_enabled(config, default_order[index]))
            continue;

        Factory factory = factories[index];
        void *module_config = get_module_config(config, default_order[index]);
        modules.push_back( factory(module_config) );
    }

    return modules; // C++17 guaranteed NRVO
}
auto makeModules(void *config) -> std::vector<std::unique_ptr<Base>>
{
    init_click_events_handling();

    std::vector<std::unique_ptr<Base>> modules;

    const char *buffer[20];
    _Static_assert(sizeof(buffer) / sizeof(const char*) >= default_order_len);

    auto *end = get_module_order(config, buffer, sizeof(buffer) / sizeof(const char*));
    if (end) {
        std::size_t len = end - buffer;

        std::size_t index_buffer[20];
        for (std::size_t i = 0; i != len; ++i) {
            auto it = std::find_if(
                default_order, default_order + default_order_len,
                [&](const char *elem)
                {
                    return std::strcmp(buffer[i], elem) == 0;
                }
            );
            if (it == default_order + default_order_len)
                errx(1, "Unknown module name %s", buffer[i]);

            index_buffer[i] = it - default_order;
        }

        modules = makeModules(config, index_buffer, len);
    } else
        modules = makeModules(config, default_index, default_index_len);

    return modules;
}
} /* namespace swaystatus::modules */
