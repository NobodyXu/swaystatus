#ifndef  _GNU_SOURCE
# define _GNU_SOURCE     /* For strchrnul, strcasestr */
#endif

#include <cstddef>
#include <cstring>

#include <err.h>

#include <fcntl.h>     /* For O_RDONLY */
#include <unistd.h>    /* For close, lseek and fstat */

#include <utility>

#include "formatting/fmt/include/fmt/core.h"

#include "utility.h"
#include "formatting/fmt_utility.hpp"

#include "Battery.hpp"

namespace swaystatus {
auto Battery::makeBattery(int path_fd, std::string_view device, std::string_view excluded_model) ->
    std::optional<Battery>
{
    std::size_t device_name_len = device.size();

    std::string path(device);
    path.reserve(device_name_len + 1 + sizeof("uevent"));

    path.append("/type");

    int fd = openat_checked(power_supply_path, path_fd, path.c_str(), O_RDONLY);

    char type[sizeof("Battery")];
    ssize_t bytes = readall(fd, type, sizeof(type) - 1);
    if (bytes < 0)
        err(1, "%s on %s%s failed", "readall", power_supply_path, path.c_str());
    if (bytes == 0)
        errx(1, "%s on %s%s failed", "Assumption", power_supply_path, path.c_str());
    type[bytes] = '\0';

    close(fd);

    if (std::strncmp("Battery", type, sizeof("Battery") - 1) != 0)
        return {std::nullopt};

    path.resize(device_name_len);

    auto battery = Battery{path_fd, std::move(path)};

    battery.read_battery_uevent();
    if (battery.get_property("model_name") == excluded_model)
        return std::nullopt;

    return battery;
}

Battery::Battery(int path_fd, std::string &&device):
    battery_device{std::move(device)}
{
    auto device_sz = battery_device.size();

    auto &buffer = battery_device;
    buffer.append("/uevent");
    uevent_fd = openat_checked(power_supply_path, path_fd, buffer.c_str(), O_RDONLY);
    battery_device.resize(device_sz);
    battery_device.shrink_to_fit();
}

void Battery::read_battery_uevent()
{
    buffer.clear();

    ssize_t cnt = asreadall(uevent_fd.get(), buffer);
    if (cnt < 0)
        err(1, "%s on %s%s/%s failed", "read", power_supply_path, battery_device.c_str(), "uevent");

    if (lseek(uevent_fd.get(), 0, SEEK_SET) == static_cast<off_t>(-1))
        err(1, "%s on %s%s/%s failed", "lseek", power_supply_path, battery_device.c_str(), "uevent");
}

auto Battery::get_property(std::string_view name) const noexcept -> std::string_view
{
    if (!uevent_fd)
        return {};

    std::size_t name_len = name.size();

    char *substr = const_cast<char*>(buffer.c_str());
    for (; ;) {
        substr = strcasestr(substr, name.data());
        if (!substr)
            return "nullptr";
        if (substr[name_len] == '=')
            break;
        substr += name_len;
    }

    const char *value = substr + name_len + 1;
    const char *end = strchrnul(substr, '\n');

    return {value, static_cast<std::size_t>(end - value)};
}
} /* namespace swaystatus */

using Batteries_formatter = fmt::formatter<std::vector<swaystatus::Battery>>;

auto Batteries_formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    end = swaystatus::find_end_of_format(ctx);

    fmt_str = std::string_view{it, static_cast<std::size_t>(end - it)};

    return end;
}
auto Batteries_formatter::format(const Batteries &batteries, format_context &ctx)
    -> format_context_it
{
    auto out = ctx.out();

    if (fmt_str.size() == 0)
        return out;

    for (const swaystatus::Battery &battery: batteries) {
        auto get_bat_property_lazy = [&battery](std::string_view name) noexcept
        {
            return swaystatus::LazyEval{[&battery, name]() noexcept
            {
                return battery.get_property(name);
            }};
        };
        auto get_conditional_lazy = [&battery](std::string_view name, std::string_view val) noexcept
        {
            return swaystatus::LazyEval{[&battery, name, val]() noexcept
            {
                return swaystatus::Conditional{battery.get_property(name) == val};
            }};
        };

        out = format_to(
            out,

            fmt_str,

            fmt::arg("type", "battery"),

#define ARG(literal) fmt::arg((literal), get_bat_property_lazy(literal))
            ARG("name"),
            ARG("present"),
            ARG("technology"),

            ARG("model_name"),
            ARG("manufacturer"),
            ARG("serial_number"),

            ARG("status"),

            ARG("cycle_count"),

            ARG("voltage_min_design"),
            ARG("voltage_now"),

            ARG("charge_full_design"),
            ARG("charge_full"),
            ARG("charge_now"),

            ARG("capacity"),
            ARG("capacity_level"),
#undef ARG

            fmt::arg("is_charging", get_conditional_lazy("status", "Charging")),
            fmt::arg("is_discharging", get_conditional_lazy("status", "Discharging")),
            fmt::arg("is_not_charging", get_conditional_lazy("status", "Not charging")),
            fmt::arg("is_full", get_conditional_lazy("status", "Full"))
        );
    }

    return out;
}
