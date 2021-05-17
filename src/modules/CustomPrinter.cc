#include "CustomPrinter.hpp"

namespace swaystatus::modules {
class CustomPrinter: public Base {
    ;

public:
    CustomPrinter(void *config):
        Base{config, "custom", 1, nullptr, nullptr, "callback"}
    {
        ;
    }

    ~CustomPrinter() = default;

    void update() {
        ;
    }
    void do_print(const char *format) {
        ;
    }
    void reload()
    {
        ;
    }
};

std::unique_ptr<Base> makeCustomPrinter(void *config) {
    return std::make_unique<CustomPrinter>(config);
}
} /* namespace swaystatus::modules */
