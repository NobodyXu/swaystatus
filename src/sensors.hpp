#ifndef  __swaystatus_sensors_HPP__
# define __swaystatus_sensors_HPP__

# include "formatting/fmt_config.hpp"

# include <cstdint>
# include <limits>
# include <string>
# include <string_view>
# include <array>
# include <vector>

# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
struct sensor_bus_type {
    short val;
};
struct sensor_bus_id {
    sensor_bus_type type;
    short nr;
};

class Sensors;
struct Sensor {
    std::string prefix;
    std::string path;
    int addr;

    sensor_bus_id bus;

    Sensor(const char *prefix, const char *path, int addr, short type, short nr) noexcept;

    void update(Sensors &sensors);
    auto get_adapter_name() const noexcept -> std::string_view;
};

struct sensor_reading {
    Sensor *sensor;

    int number;

    sensor_reading(Sensor *sensor, int number) noexcept;

    /**
     * It is unlikely that a compuer can run under temperature outside of range
     * [-126, 125]
     *
     * It might be a good idea in future to add field temp_critical_alarm so that
     * swaystatus can set urgent according to that
     *
     * There's also other interesting meteric, listed in /usr/include/sensors/sensors.h
     */
    std::int8_t temp = std::numeric_limits<std::int8_t>::min();
};

class Sensors {
    std::vector<Sensor> sensors;
    std::vector<sensor_reading> readings;

    friend Sensor;

public:
    Sensors() = default;

    /**
     * Initialize libsensors and get a list of sensors on the system and fetch their value.
     */
    void init();

    void update();

    using const_iterator = std::vector<sensor_reading>::const_iterator;

    auto begin() const noexcept -> const_iterator;
    auto end() const noexcept -> const_iterator;

    auto cbegin() const noexcept -> const_iterator;
    auto cend() const noexcept -> const_iterator;
};
} /* namespace swaystatus */

template <>
struct fmt::formatter<swaystatus::sensor_bus_type>:
    fmt::formatter<std::string_view>
{
    using sensor_bus_type = swaystatus::sensor_bus_type;

    using format_context_it = typename format_context::iterator;

    auto format(const sensor_bus_type &type, format_context &ctx) -> format_context_it;
};

#endif
