#define _DEFAULT_SOURCE /* For setlintbuf */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <err.h>

#include "help.h"
#include "process_configuration.h"

#include "print_battery.h"
#include "print_time.h"
#include "print_volume.h"
#include "print_network_interfaces.h"
#include "print_brightness.h"
#include "print_memory_usage.h"
#include "print_load.h"

static void parse_cmdline_arg_and_initialize(
    int argc, char* argv[],
    struct JSON_elements_strs *elements
)
{
    void *config = NULL;

    for (int i = 1; i != argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            errx(1, help);
        } else {
            if (config)
                errx(1, "Error: configuration file is specified twice");
            config = load_config(argv[i]);
        }
    }

    /*
     * Make sure fflush only happens when a newline is outputed.
     * Otherwise, swaybar might misbehave.
     */
    setlinebuf(stdout); 

    init_time(get_format(config, "time", "%Y-%m-%d %T"));
    init_upclient();
    init_alsa(
        get_property(config, "volume", "mix_name", "Master"),
        get_property(config, "volume", "card",     "default")
    );
    init_network_interfaces_scanning();
    init_brightness_detection();
    init_memory_usage_collection();
    init_load();

    config2json_elements_strs(config, elements);

    free_config(config);
}

static void print_block(void (*print)(), const char *json_element_str)
{
    fputs("{\"full_text\":\"", stdout);
    print();
    fputs("\",", stdout);
    fputs(json_element_str, stdout);
    fputs("}", stdout);
}
static void print_delimiter()
{
    fputs(",", stdout);
}

int main(int argc, char* argv[])
{
    struct JSON_elements_strs elements;
    parse_cmdline_arg_and_initialize(argc, argv, &elements);

    /* Print header */
    puts("{\"version\":1}");
    /* Begin an infinite array */
    puts("[");

    for ( ; ; sleep(1)) {
        fputs("[", stdout);

        print_block(print_brightness, elements.brightness);
        print_delimiter();

        print_block(print_volume, elements.volume);
        print_delimiter();

        print_block(print_battery, elements.battery);
        print_delimiter();

        print_block(print_network_interfaces, elements.network_interface);
        print_delimiter();

        print_block(print_load, elements.load);
        print_delimiter();

        print_block(print_memory_usage, elements.memory_usage);
        print_delimiter();

        print_block(print_time, elements.time);

        puts("],");
    }

    return 0;
}
