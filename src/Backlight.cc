#define _DEFAULT_SOURCE /* For macro constants of struct dirent::d_type and struct timespec */

#include <err.h>

#include <fcntl.h>     /* For O_RDONLY */
#include <unistd.h>    /* For close and lseek */

#include "utility.h"

#include "Backlight.hpp"

namespace swaystatus {
Backlight::Backlight(int path_fd, const char *filename_arg):
    filename{filename_arg}
{
    auto &buffer = filename;
    auto filename_sz = filename.size();

    {
        buffer.append("/max_brightness");
        int fd = openat_checked(path, path_fd, buffer.c_str(), O_RDONLY);
        buffer.resize(filename_sz);

        const char *failed_part = readall_as_uintmax(fd, &max_brightness);
        if (failed_part)
            err(1, "%s on %s%s/%s failed", failed_part, path, buffer.c_str(), "max_brightness");
        close(fd);
    }

    buffer.append("/brightness");
    this->fd = openat_checked(path, path_fd, buffer.c_str(), O_RDONLY);
    buffer.resize(filename_sz);

    buffer.shrink_to_fit();
}

Backlight::~Backlight()
{
    close(fd);
}

std::uintmax_t Backlight::calculate_brightness()
{
    uintmax_t val;
    const char *failed_part = readall_as_uintmax(fd, &val);
    if (failed_part)
        err(1, "%s on %s%s/%s failed", failed_part, path, filename.c_str(), "brightness");

    if (lseek(fd, 0, SEEK_SET) == (off_t) -1)
        err(1, "%s on %s%s/%s failed", "lseek", path, filename.c_str(), "brightness");

    return 100 * val / max_brightness;
}
void Backlight::update_brightness()
{
    brightness = calculate_brightness();
}

auto Backlight::get_device_name() const noexcept -> std::string_view
{
    return filename;
}
auto Backlight::get_brightness() const noexcept -> std::uintmax_t
{
    return brightness;
}
auto Backlight::get_max_brightness() const noexcept -> std::uintmax_t
{
    return max_brightness;
}
} /* namespace swaystatus */
