#define _DEFAULT_SOURCE /* For macro constants of struct dirent::d_type */
#ifndef  _GNU_SOURCE
# define _GNU_SOURCE     /* For strchrnul */
#endif

#include <dirent.h>

#include <err.h>

#include <vector>

#include "../utility.h"
#include "../process_configuration.h"
#include "../Battery.hpp"

#include "BatteryPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class BatteryPrinter: public Base {
    std::vector<Battery> batteries;

public:
    BatteryPrinter(void *config, const char *excluded_model):
        Base{
            config, "BatteryPrinter"sv,
            3, "{has_battery:{per_battery_fmt_str:{status} {capacity}%}}", nullptr,
            "excluded_model"
        }
    {
        DIR *dir = opendir(Battery::power_supply_path);
        if (!dir)
            err(1, "%s on %s failed", "opendir", Battery::power_supply_path);

        const int path_fd = dirfd(dir);

        errno = 0;
        for (struct dirent *ent; (ent = readdir(dir)); errno = 0) {
            switch (ent->d_type) {
                case DT_UNKNOWN:
                case DT_LNK:
                    if (!isdir(Battery::power_supply_path, path_fd, ent->d_name))
                        break;

                case DT_DIR:
                    if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                        auto result = Battery::makeBattery(path_fd, ent->d_name, excluded_model);
                        if (result)
                            batteries.push_back(std::move(*result));
                    }

                default:
                    break;
            }
        }

        if (errno != 0)
            err(1, "%s on %s failed", "readdir", Battery::power_supply_path);

        batteries.shrink_to_fit();

        closedir(dir);
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
};

std::unique_ptr<Base> makeBatteryPrinter(void *config)
{
    std::unique_ptr<const char[]> excluded_model{get_property(config, "excluded_model", nullptr)};

    return std::make_unique<BatteryPrinter>(
        config,
        excluded_model.get()
    );
}
} /* namespace swaystatus::modules */
