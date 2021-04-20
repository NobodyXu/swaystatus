#ifndef  __swaystatus_Battery_HPP__
# define __swaystatus_Battery_HPP__

# include <string>
# include <string_view>
# include <vector>
# include <optional>

# include "LazyEval.hpp"
# include "Conditional.hpp"

# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
class Battery {
    std::string battery_device;
    int uevent_fd;

    std::string buffer;

protected:
    Battery(int path_fd, std::string &&battery_device);

public:
    static constexpr const auto *power_supply_path = "/sys/class/power_supply/";

    static auto makeBattery(int path_fd, std::string_view device) -> std::optional<Battery>;

    Battery(Battery &&) noexcept;

    Battery& operator = (const Battery&) = delete;
    Battery& operator = (Battery&&) = delete;

    ~Battery();

    void read_battery_uevent();

    auto get_property(std::string_view name) const noexcept -> std::string_view;
};

class Batteries {
    std::vector<Battery> batteries;

public:
    Batteries() = default;
};
} /* namespace swaystatus */

template <>
struct fmt::formatter<std::vector<swaystatus::Battery>>
{
    using Batteries = std::vector<swaystatus::Battery>;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::string_view fmt_str = "";

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const Batteries &batteries, format_context &ctx) -> format_context_it;
};

#endif
