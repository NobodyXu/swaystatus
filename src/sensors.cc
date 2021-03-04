/**
 * Adapted from:
 * https://stackoverflow.com/questions/8556551/has-anyone-been-able-to-use-libsensors-properly
 */
#include <cmath>
#include <err.h>

#include <sensors/sensors.h>

#include "sensors.hpp"

namespace swaystatus {
static std::int8_t sensor_get_int8_val(const sensors_chip_name &cn, const sensors_subfeature *subf)
{
    double val;
    if (sensors_get_value(&cn, subf->number, &val) < 0)
        errx(1, "%s failed", "sensors_get_value");

    return static_cast<std::int8_t>(std::lround(val));
}

Sensor::Sensor(const char *prefix, const char *path, int addr, short type, short nr) noexcept:
    prefix{prefix}, path{path}, addr{addr}, bus{sensor_bus_type{type}, nr}
{}

auto Sensor::get_adapter_name() const noexcept -> std::string_view
{
    const sensors_bus_id bus_id = {
        .type = bus.type.val,
        .nr = bus.nr
    };
    const char *ret = sensors_get_adapter_name(&bus_id);
    if (ret)
        return ret;
    else
        return {};
}
void Sensor::update()
{
    reading_cnt = 0;

    /*
     * The const_cast is used only to maintain compatibility with the sensors_bus_id defined in
     * sensors/sensors.h
     */
    const sensors_chip_name cn = {
        .prefix = const_cast<char*>(prefix.c_str()),
        .bus = sensors_bus_id{
            .type = bus.type.val,
            .nr = bus.nr
        },
        .addr = addr,
        .path = const_cast<char*>(path.c_str())
    };

    sensors_feature const *feat;
    int f = 0;

    while ((feat = sensors_get_features(&cn, &f)) != 0) {
        if (feat->type != SENSORS_FEATURE_TEMP)
            continue;

        if (reading_cnt == readings.size())
            break;
        auto &reading = readings[reading_cnt];
        reading.number = feat->number;
        ++reading_cnt;

        sensors_subfeature const *subf;
        int s = 0;

        std::uint8_t cnt = 0;
        while ((subf = sensors_get_all_subfeatures(&cn, feat, &s)) != 0) {
            if ((subf->flags & SENSORS_MODE_R) == 0)
                continue;

            std::int8_t *result;
            switch (subf->type) {
                default:
                    continue;

                case SENSORS_SUBFEATURE_TEMP_INPUT:
                    result = &reading.temp;
                    break;
            }

            *result = sensor_get_int8_val(cn, subf);
            ++cnt;
        }

        /**
         * Check if the reading contains any valid data
         *
         * On my computer, BAT0 always return subfeatures that have no TEMP_INPUT,
         * so this is necessary.
         */
        if (cnt == 0)
            --reading_cnt;
    }
}

void Sensors::init()
{
    if (sensors_init(nullptr) != 0)
        errx(1, "%s failed", "sensors_init");

    sensors_chip_name const * cn;
    int c = 0;
    while ((cn = sensors_get_detected_chips(NULL, &c)) != 0) {
        const auto &bus = cn->bus;
        sensors.emplace_back(cn->prefix, cn->path, cn->addr, bus.type, bus.nr);
    }

    sensors.shrink_to_fit();

    update();
}

void Sensors::update()
{
    for (auto &sensor: sensors)
        sensor.update();
}

auto Sensors::begin() const noexcept -> const_iterator
{
    return sensors.begin();
}
auto Sensors::end() const noexcept -> const_iterator
{
    return sensors.end();
}

auto Sensors::cbegin() const noexcept -> const_iterator
{
    return sensors.begin();
}
auto Sensors::cend() const noexcept -> const_iterator
{
    return sensors.end();
}
} /* namespace swaystatus */

using formatter = fmt::formatter<swaystatus::sensor_bus_type>;

static auto bus_type_to_str(const swaystatus::sensor_bus_type &type) noexcept -> std::string_view
{
    switch (type.val) {
#define CASE(name) case SENSORS_BUS_TYPE_ ## name: return (# name)

        CASE(ANY);
        CASE(I2C);
        CASE(ISA);
        CASE(PCI);
        CASE(SPI);
        CASE(VIRTUAL);
        CASE(ACPI);
        CASE(HID);
        CASE(MDIO);
        CASE(SCSI);

#undef  CASE

        default:
            return "UNKNOWN";
    }
}
auto formatter::format(const sensor_bus_type &type, format_context &ctx) -> format_context_it
{
    return fmt::formatter<std::string_view>::format(bus_type_to_str(type), ctx);
}
