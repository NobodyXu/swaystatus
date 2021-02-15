#define _DEFAULT_SOURCE         /* For macro constants of struct dirent::d_type */

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>     /* For O_RDONLY */
#include <unistd.h>    /* For close and lseek */

#include "utility.h"
#include "print_brightness.h"

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
};

static struct Backlight *backlights;
static size_t backlight_sz;

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
}

void init_brightness_detection()
{
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
        errx(1, "No %s dir is found", path);

    closedir(dir);
}

void print_brightness()
{
    for (size_t i = 0; i != backlight_sz; ++i) {
        const struct Backlight * const backlight = &backlights[i];

        uintmax_t brightness;
        const char *failed_part = readall_as_uintmax(backlight->fd, &brightness);
        if (failed_part)
            err(1, "%s on %s%s/%s failed",
                    failed_part, path, backlight->filename, "max_brightness");

        printf("%s: %" PRIuMAX, backlight->filename, 100 * brightness / backlight->max_brightness);

        if (i + 1 != backlight_sz)
            fputs(" ", stdout);
        if (lseek(backlight->fd, 0, SEEK_SET) == (off_t) -1)
            err(1, "%s on %s%s/%s failed", "lseek", path, backlight->filename, "brightness");
    }
}
