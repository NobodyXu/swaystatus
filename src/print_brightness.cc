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

#include "utility.h"
#include "printer.hpp"
#include "Conditional.hpp"
#include "print_brightness.h"

using swaystatus::Conditional;

static const char * const path = "/sys/class/backlight/";

struct Backlight {
    const char *filename;
    /**
     * value read from max_brightness
     */
    uintmax_t max_brightness;
    /**
     * opened file of /sys/class/backlight/{BacklightDevice}/brightness
     */
    int fd;
    /**
     * cached brightness
     */
    uintmax_t brightness;
};

static struct Backlight *backlights;
static size_t backlight_sz;

static const char *format;

static uint32_t cycle_cnt;
static uint32_t interval;

extern "C" {
static void update_brightness(struct Backlight *backlight);
static void addBacklight(int path_fd, const char *filename)
{
    ++backlight_sz;
    reallocate(backlights, backlight_sz);

    struct Backlight *backlight = &backlights[backlight_sz - 1];
    backlight->filename = strdup_checked(filename);

    size_t filename_sz = strlen(backlight->filename);

    char buffer[filename_sz + 1 + sizeof("max_brightness")];

    /*
     * Accessed linearly from low addr to high addr, thus no need to worry about
     * overwritng heap.
     */
    memcpy(buffer, backlight->filename, filename_sz);
    buffer[filename_sz] = '/';
    memcpy(buffer + filename_sz + 1, "max_brightness", sizeof("max_brightness"));

    int fd = openat_checked(path, path_fd, buffer, O_RDONLY);

    const char *failed_part = readall_as_uintmax(fd, &backlight->max_brightness);
    if (failed_part)
        err(1, "%s on %s%s/%s failed", failed_part, path, backlight->filename, "max_brightness");
    close(fd);

    memcpy(buffer + filename_sz + 1, "brightness", sizeof("brightness"));
    backlight->fd = openat_checked(path, path_fd, buffer, O_RDONLY);

    update_brightness(backlight);
}

void init_brightness_detection(const char *format_str, uint32_t interval_arg)
{
    format = format_str;
    interval = interval_arg;

    DIR *dir = opendir(path);
    if (!dir)
        err(1, "%s on %s failed", "opendir", path);

    const int path_fd = dirfd(dir);

    errno = 0;
    for (struct dirent *ent; (ent = readdir(dir)); errno = 0) {
        switch (ent->d_type) {
            case DT_UNKNOWN:
            case DT_LNK:
                if (!isdir(path, path_fd, ent->d_name))
                    break;

            case DT_DIR:
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    addBacklight(path_fd, ent->d_name);

            default:
                break;
        }
    }

    if (errno != 0)
        err(1, "%s on %s failed", "readdir", path);

    if (backlight_sz == 0)
        errx(1, "No dir is found under %s", path);

    closedir(dir);
}

static uintmax_t calculate_brightness(struct Backlight *backlight)
{
    uintmax_t val;
    const char *failed_part = readall_as_uintmax(backlight->fd, &val);
    if (failed_part)
        err(1, "%s on %s%s/%s failed", failed_part, path, backlight->filename, "brightness");

    if (lseek(backlight->fd, 0, SEEK_SET) == (off_t) -1)
        err(1, "%s on %s%s/%s failed", "lseek", path, backlight->filename, "brightness");

    return 100 * val / backlight->max_brightness;
}
static void update_brightness(struct Backlight *backlight)
{
    backlight->brightness = calculate_brightness(backlight);
}

void print_brightness()
{
    ++cycle_cnt;

    for (size_t i = 0; i != backlight_sz; ++i) {
        struct Backlight * const backlight = &backlights[i];

        if (cycle_cnt == interval)
            update_brightness(backlight);

        swaystatus::print(
            format,
            fmt::arg("backlight_device", backlight->filename),
            fmt::arg("brightness",       backlight->brightness),
            fmt::arg("has_multiple_backlight_devices", Conditional{backlight_sz != 1})
        );

        if (i + 1 != backlight_sz)
            print_literal_str(" ");
    }

    if (cycle_cnt == interval)
        cycle_cnt = 0;
}
}
