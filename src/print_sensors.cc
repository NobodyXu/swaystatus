#include <err.h>
#include <exception>

#include "sensors.hpp"
#include "process_configuration.h"
#include "handle_click_events.h"
#include "printer.hpp"
#include "print_sensors.h"

using swaystatus::Sensors;
using swaystatus::print;
using swaystatus::get_user_specified_property_str;

static const char * const module_name = "sensors";

static const char *user_specified_properties_str;

static const char *full_text_format;
static const char *short_text_format;

static uint32_t cycle_cnt;
static uint32_t interval;

static Sensors sensors;
static typename Sensors::const_iterator reading_it;

extern "C" {
void init_sensors(void *config)
{
    full_text_format = get_format(
        config,
        "{prefix} {reading_number}th sensor: {reading_temp}°C"
    );
    short_text_format = get_short_format(config, NULL);
    interval = get_update_interval(config, "sensors", 5);

    add_click_event_handler(module_name, get_click_event_handler(config));

    user_specified_properties_str = get_user_specified_property_str(config);

    sensors.init();
    reading_it = sensors.begin();
}

static void print_fmt(const char *name, const char *format)
{
    print("\"{}\":\"", name);

    auto &sensor = *reading_it->sensor;
    auto &bus = sensor.bus;
    auto &reading = *reading_it;

    try {
        print(
            format,
            fmt::arg("prefix",   sensor.prefix),
            fmt::arg("path",     sensor.path),
            fmt::arg("addr",     sensor.addr),
            fmt::arg("bus_type", bus.type),
            fmt::arg("bus_nr",   bus.nr),

            fmt::arg("reading_number", reading.number),
            fmt::arg("reading_temp",   reading.temp)
        );
    } catch (const std::exception &e) {
        errx(1, "Failed to print %s format in print_%s.cc: %s", name, "sensors", e.what());
    }

    print_literal_str("\",");
}
void print_sensors()
{
    if (++cycle_cnt == interval) {
        cycle_cnt = 0;

        ++reading_it;
        if (reading_it == sensors.end()) {
            sensors.update();
            reading_it = sensors.begin();
        }
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
