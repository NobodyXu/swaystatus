#include <cstddef>
#include <vector>

#include "../utility.h"
#include "../Backlight.hpp"
#include "../formatting/Conditional.hpp"

#include "BacklightPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class BacklightPrinter: public Base {
    std::vector<Backlight> backlights;

public:
    BacklightPrinter(void *config):
        Base{
            config, "BacklightPrinter"sv,
            1, "{backlight_device}: {brightness}", nullptr
        }
    {
        visit_all_subdirs(
            Backlight::path,
            [](int path_fd, const char *d_name, va_list ap)
            {
                va_list args;
                va_copy(args, ap);
                va_arg(args, std::vector<Backlight>*)->emplace_back(path_fd, d_name);
                va_end(args);
            },
            &backlights
        );

        backlights.shrink_to_fit();
    }

    ~BacklightPrinter() = default;

    void update()
    {
        for (Backlight &backlight: backlights)
            backlight.update_brightness();
    }
    void do_print(const char *format)
    {
        std::size_t i = 0;
        for (const Backlight &backlight: backlights) {
            print(
                format,
                fmt::arg("backlight_device", backlight.get_device_name()),
                fmt::arg("brightness",       backlight.get_brightness()),
                fmt::arg("max_brightness",   backlight.get_max_brightness()),
                fmt::arg("has_multiple_backlight_devices", Conditional{backlights.size() != 1})
            );

            if (i++ != backlights.size())
                print_literal_str(" ");
        }
    }
};

std::unique_ptr<Base> makeBacklightPrinter(void *config)
{
    return std::make_unique<BacklightPrinter>(config);
}
} /* namespace swaystatus::modules */
