#define _DEFAULT_SOURCE /* For macro constants of struct dirent::d_type */
#ifndef  _GNU_SOURCE
# define _GNU_SOURCE     /* For strchrnul */
#endif

#include <vector>

#include "../utility.h"
#include "../process_configuration.h"
#include "../Battery.hpp"

#include "BatteryPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class BatteryPrinter: public Base {
    std::unique_ptr<const char[]> excluded_model;
    std::vector<Battery> batteries;

    void load()
    {
        std::string_view excluded_model_sv;
        if (excluded_model)
            excluded_model_sv = excluded_model.get();

        visit_all_subdirs(
            Battery::power_supply_path,
            [](int path_fd, const char *d_name, va_list ap)
            {
                va_list args;
                va_copy(args, ap);

                auto &excluded_model = *va_arg(args, std::string_view*);
                auto &batteries = *va_arg(args, std::vector<Battery>*);

                auto result = Battery::makeBattery(path_fd, d_name, excluded_model);
                if (result)
                    batteries.push_back(std::move(*result));

                va_end(args);
            },
            &excluded_model_sv, &batteries
        );

        batteries.shrink_to_fit();
    }

public:
    BatteryPrinter(void *config, std::unique_ptr<const char[]> &&excluded_model_arg):
        Base{
            config, "BatteryPrinter"sv,
            3, "{has_battery:{per_battery_fmt_str:{status} {capacity}%}}", nullptr,
            "excluded_model"
        },
        excluded_model{std::move(excluded_model_arg)}
    {
        load();
    }

    ~BatteryPrinter() = default;

    void update()
    {
        for (Battery &bat: batteries)
            bat.read_battery_uevent();
    }
    void do_print(const char *format)
    {
        print(
            format, 
            fmt::arg("has_battery", swaystatus::Conditional{batteries.size() != 0}),
            fmt::arg("per_battery_fmt_str", batteries)
        );
    }
    void reload()
    {
        batteries.clear();
        load();
    }
};

std::unique_ptr<Base> makeBatteryPrinter(void *config)
{
    return std::make_unique<BatteryPrinter>(
        config,
        std::unique_ptr<const char[]>{get_property(config, "excluded_model", nullptr)}
    );
}
} /* namespace swaystatus::modules */
