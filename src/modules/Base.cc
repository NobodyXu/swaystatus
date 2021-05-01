#include <err.h>

#include <cstdarg>
#include <cstring>

#include "../error_handling.hpp"
#include "../process_configuration.h"
#include "../handle_click_events.h"
#include "../formatting/printer.hpp"

#include "Base.hpp"

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
    interval{get_update_interval(config, module_name_arg.data(), default_interval)}
{
    this->cycle_cnt = interval - 1;

    add_click_event_handler(module_name.data(), get_click_event_handler(config));

    std::va_list ap;
    va_start(ap, n);
    user_specified_properties_str.reset(get_user_specified_property_str_impl2(config, n, ap));
    if (user_specified_properties_str)
        user_specified_properties_str_len = std::strlen(user_specified_properties_str.get());
    va_end(ap);
}

void Base::update_and_print()
{
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
        do_print(format);
    } CATCH (const std::exception &e) {
        errx(1, "Failed to print %s format in %s: %s",
                name.data(), module_name.data(), e.what());
    };

    print_literal_str("\",");
}
} /* namespace swaystatus::modules */
