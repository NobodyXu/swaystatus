#include "sensors.hpp"
#include "printer.hpp"
#include "print_sensors.h"

using swaystatus::Sensors;

static const char *format;

static uint32_t cycle_cnt;
static uint32_t interval;

static Sensors sensors;
static typename Sensors::const_iterator sensor_it;
static std::uint8_t reading_index = 0;

extern "C" {
void init_sensors(const char *format_arg, uint32_t interval_arg)
{
    format = format_arg;
    interval = interval_arg;

    sensors.init();
    sensor_it = sensors.begin();
}

void print_sensors()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;

        ++reading_index;
        if (reading_index == sensor_it->reading_cnt) {
            reading_index = 0;
            ++sensor_it;
            if (sensor_it == sensors.end()) {
                sensors.update();
                sensor_it = sensors.begin();
            }
        }
    }

    auto &sensor = *sensor_it;
    auto &bus = sensor.bus;
    auto &reading = sensor.readings[reading_index];

    swaystatus::print(
        format,
        fmt::arg("prefix",   sensor.prefix),
        fmt::arg("path",     sensor.path),
        fmt::arg("addr",     sensor.addr),
        fmt::arg("bus_type", bus.type),
        fmt::arg("bus_nr",   bus.nr),

        fmt::arg("reading_number", reading.number),
        fmt::arg("reading_temp",   reading.temp)
    );
}
}
