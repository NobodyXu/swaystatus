#define _DEFAULT_SOURCE /* For macro constants of struct dirent::d_type and struct timespec */

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>     /* For O_RDONLY */
#include <unistd.h>    /* For close and lseek */

#include <exception>
#include <vector>

#include "../error_handling.hpp"
#include "../utility.h"
#include "../process_configuration.h"
#include "../handle_click_events.h"
#include "../formatting/printer.hpp"
#include "../formatting/Conditional.hpp"
#include "../Backlight.hpp"
#include "print_brightness.h"

using swaystatus::Conditional;
using swaystatus::print;
using swaystatus::get_user_specified_property_str;
using swaystatus::Backlight;

static const char * const module_name = "brightness";

static std::vector<Backlight> backlights;

static const char *user_specified_properties_str;

static const char *full_text_format;
static const char *short_text_format;

static uint32_t cycle_cnt;
static uint32_t interval;

extern "C" {
void init_brightness_detection(void *config)
{
    full_text_format = get_format(config, "{backlight_device}: {brightness}");
    short_text_format = get_short_format(config, NULL);

    interval = get_update_interval(config, "brightness", 1);

    add_click_event_handler(module_name, get_click_event_handler(config));

    user_specified_properties_str = get_user_specified_property_str(config);

    DIR *dir = opendir(Backlight::path);
    if (!dir)
        err(1, "%s on %s failed", "opendir", Backlight::path);

    const int path_fd = dirfd(dir);

    errno = 0;
    for (struct dirent *ent; (ent = readdir(dir)); errno = 0) {
        switch (ent->d_type) {
            case DT_UNKNOWN:
            case DT_LNK:
                if (!isdir(Backlight::path, path_fd, ent->d_name))
                    break;

            case DT_DIR:
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    backlights.emplace_back(path_fd, ent->d_name);

            default:
                break;
        }
    }

    if (errno != 0)
        err(1, "%s on %s failed", "readdir", Backlight::path);

    if (backlights.size() == 0)
        errx(1, "No dir is found under %s", Backlight::path);

    closedir(dir);
    backlights.shrink_to_fit();
}

static void print_fmt(const char *name, const char *fmt)
{
    print("\"{}\":\"", name);

    size_t i = 0;
    for (const Backlight &backlight: backlights) {
        TRY {
            print(
                fmt,
                fmt::arg("backlight_device", backlight.get_device_name()),
                fmt::arg("brightness",       backlight.get_brightness()),
                fmt::arg("max_brightness",       backlight.get_max_brightness()),
                fmt::arg("has_multiple_backlight_devices", Conditional{backlights.size() != 1})
            );
        } CATCH (const std::exception &e) {
            errx(1, "Failed to print %s format in print_%s.cc: %s", name, "brightness", e.what());
        };

        if (i++ != backlights.size())
            print_literal_str(" ");
    }

    print_literal_str("\",");
}
void print_brightness()
{
    ++cycle_cnt;
    if (cycle_cnt == interval) {
        for (Backlight &backlight: backlights)
            backlight.update_brightness();
        cycle_cnt = 0;
    }

    print_literal_str("{\"name\":\"");
    print_str(module_name);
    print_literal_str("\",\"instance\":\"0\",");

    print_fmt("full_text", full_text_format);
    if (short_text_format)
        print_fmt("short_text", short_text_format);

    print_str(user_specified_properties_str);

    print_literal_str("},");
}
} /* extern "C" */
