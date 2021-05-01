#include <err.h>

#include "../process_configuration.h"
#include "../alsa.h"

#include "VolumePrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class VolumePrinter: public Base {
    ;

public:
    VolumePrinter(void *config, const char *mix_name, const char *card):
        Base{
            config, "VolumePrinter"sv,
            1, "vol {volume}%", nullptr,
            "mix_name", "card"
        }
    {
        initialize_alsa_lib(mix_name, card);
    }

    void update()
    {
        update_volume();
    }
    void do_print(const char *format)
    {
        print(format, fmt::arg("volume", get_audio_volume()));
    }
};

std::unique_ptr<Base> makeVolumePrinter(void *config)
{
    std::unique_ptr<const char[]> mix_name{get_property(config, "mix_name", "Master")};
    std::unique_ptr<const char[]> card    {get_property(config, "card",     "default")};

    return std::make_unique<VolumePrinter>(config, mix_name.get(), card.get());
}
} /* namespace swaystatus::modules */
