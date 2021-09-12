#include "../process_configuration.h"
#include "../Callback/Callable.hpp"

#include "CustomPrinter.hpp"

namespace swaystatus::modules {
class CustomPrinter: public Base {
    /*                   Ret type, param .. */
    swaystatus::Callable<void> update_callback;
    swaystatus::Callable<std::string> do_print_callback;

public:
    CustomPrinter(
        void *config,
        Callable_base &&update_callback_base,
        Callable_base &&do_print_callback_base
    ):
        Base{config, "custom", 1, "", nullptr, "click_event_handler", "update_callback", "do_print_callback"},
        update_callback{std::move(update_callback_base)},
        do_print_callback{std::move(do_print_callback_base)}
    {}

    ~CustomPrinter() = default;

    void update()
    {
        update_callback();
    }
    void do_print(const char *format)
    {
        (void) format;

        print_str2(do_print_callback());
    }
    void reload()
    {
        ;
    }
};

std::unique_ptr<Base> makeCustomPrinter(void *config) {
    return std::make_unique<CustomPrinter>(
        config,
        Callable_base("custom_block", get_callable(config, "update_callback")),
        Callable_base("custom_block", get_callable(config, "do_print_callback"))
    );
}
} /* namespace swaystatus::modules */
