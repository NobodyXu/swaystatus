#include "../sensors.hpp"

#include "TemperaturePrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class TemperaturePrinter: public Base {
    Sensors sensors;
    typename Sensors::const_iterator reading_it;

public:
    TemperaturePrinter(void *config):
        Base{
            config, "TemperaturePrinter"sv,
            5, "{prefix} {reading_number}th sensor: {reading_temp}°C", nullptr
        }
    {
        sensors.init();
        reading_it = sensors.begin();
    }

    void update()
    {
        ++reading_it;
        if (reading_it == sensors.end()) {
            sensors.update();
            reading_it = sensors.begin();
        }
    }
    void do_print(const char *format)
    {
        auto &sensor = *reading_it->sensor;
        auto &bus = sensor.bus;
        auto &reading = *reading_it;

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
    }
};

std::unique_ptr<Base> makeTemperaturePrinter(void *config)
{
    return std::make_unique<TemperaturePrinter>(config);
}
} /* namespace swaystatus::modules */
