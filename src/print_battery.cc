#define _DEFAULT_SOURCE /* For macro constants of struct dirent::d_type */
#ifndef  _GNU_SOURCE
# define _GNU_SOURCE     /* For strchrnul */
#endif

#include <cstddef>
#include <cstring>
#include <cassert>

#include <err.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>     /* For O_RDONLY */
#include <unistd.h>    /* For close, lseek and fstat */

#include <exception>
#include <string_view>
#include <vector>

#include "utility.h"
#include "process_configuration.h"
#include "printer.hpp"
#include "Battery.hpp"
#include "print_battery.h"

using swaystatus::Battery;
using swaystatus::print;

static const char *full_text_format;
static const char *short_text_format;

static uint32_t cycle_cnt;
static uint32_t interval;

static std::vector<Battery> batteries;

extern "C" {
void init_battery_monitor(const void *config)
{
    full_text_format = get_format(
        config,
        "{has_battery:{per_battery_fmt_str:{status} {capacity}%}}"
    );
    short_text_format = get_short_format(config, NULL);

    interval = get_update_interval(config, "battery", 3);

    const char *excluded_model = get_property(config, "excluded_model", "");

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

static void print_fmt(const char *name, const char *fmt)
{
    print("\"{}\":\"", name);

    try {
        print(
            fmt, 
            fmt::arg("has_battery", swaystatus::Conditional{batteries.size() != 0}),
            fmt::arg("per_battery_fmt_str", batteries)
        );
    } catch (const std::exception &e) {
        errx(1, "Failed to print %s format in print_%s.cc: %s", name, "battery", e.what());
    }

    print_literal_str("\",");
}
void print_battery()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;
        for (Battery &bat: batteries)
            bat.read_battery_uevent();
    }

    print_fmt("full_text", full_text_format);
    if (short_text_format)
        print_fmt("short_text", short_text_format);
}
} /* extern "C" */
