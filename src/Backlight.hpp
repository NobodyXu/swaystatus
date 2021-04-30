#ifndef  __swaystatus_Backlight_HPP__
# define __swaystatus_Backlight_HPP__

# include <cinttypes>
# include <string>
# include <string_view>

namespace swaystatus {
class Backlight {
public:
    static constexpr const char * const path = "/sys/class/backlight/";

    // instance variables
    std::string filename;
    /**
     * value read from max_brightness
     */
    std::uintmax_t max_brightness;
    /**
     * opened file of /sys/class/backlight/{BacklightDevice}/brightness
     */
    int fd;
    /**
     * cached brightness
     */
    std::uintmax_t brightness;

    // methods
    std::uintmax_t calculate_brightness();

public:
    Backlight() = delete;
    
    Backlight(int path_fd, const char *filename_arg);

    Backlight(const Backlight&) = delete;
    Backlight(Backlight&&) = default;

    Backlight& operator = (const Backlight&) = delete;
    Backlight& operator = (Backlight&&) = delete;

    ~Backlight();

    void update_brightness();

    auto get_device_name() const noexcept -> std::string_view;
    auto get_brightness() const noexcept -> std::uintmax_t;
    auto get_max_brightness() const noexcept -> std::uintmax_t;
};
} /* namespace swaystatus */

#endif
