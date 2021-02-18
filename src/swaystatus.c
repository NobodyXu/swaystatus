#define _DEFAULT_SOURCE /* For setlintbuf and nice */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <err.h>
#include <errno.h>

#include "help.h"
#include "utility.h"
#include "process_configuration.h"

#include "print_battery.h"
#include "print_time.h"
#include "print_volume.h"
#include "print_network_interfaces.h"
#include "print_brightness.h"
#include "print_memory_usage.h"
#include "print_load.h"

#define starts_with(str, prefix) (strncmp((str), (prefix), sizeof(prefix) - 1) == 0)

static uintmax_t parse_cmdline_arg_and_initialize(
    int argc, char* argv[],
    struct Features *features,
    struct JSON_elements_strs *elements
)
{
    /* Default interval is 1 second */
    uintmax_t interval = 1000;

    void *config = NULL;

    for (int i = 1; i != argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            errx(1, help);
        } else if (starts_with(argv[i], "--interval=")) {
            char *endptr;
            errno = 0;
            interval = strtoumax(argv[i] + sizeof("--interval=") - 1, &endptr, 10);
            if (errno == ERANGE)
                err(1, "Invalid argument %s%s", argv[i], "");
            else if (*endptr != '\0')
                errx(1, "Invalid argument %s%s", argv[i], ": Contains non-digit character");
        } else {
            if (config)
                errx(1, "Error: configuration file is specified twice");
            config = load_config(argv[i]);
        }
    }

    config2features(config, features);

    /*
     * Make sure fflush only happens when a newline is outputed.
     * Otherwise, swaybar might misbehave.
     */
    setlinebuf(stdout); 

    if (features->time)
        init_time(get_format(config, "time", "%Y-%m-%d %T"));
    if (features->battery)
        init_upclient();
    if (features->volume)
        init_alsa(
            get_property(config, "volume", "mix_name", "Master"),
            get_property(config, "volume", "card",     "default")
        );
    if (features->network_interface)
        init_network_interfaces_scanning();
    if (features->brightness)
        init_brightness_detection();
    if (features->memory_usage)
        init_memory_usage_collection();
    if (features->load)
        init_load();

    config2json_elements_strs(config, elements);

    free_config(config);

    return interval;
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
    nice(19);

    struct Features features;
    struct JSON_elements_strs elements;
    const uintmax_t interval = parse_cmdline_arg_and_initialize(argc, argv, &features, &elements);

    /* Print header */
    puts("{\"version\":1}");
    /* Begin an infinite array */
    puts("[");

    for ( ; ; msleep(interval)) {
        fputs("[", stdout);

        if (features.brightness) {
            print_block(print_brightness, elements.brightness);
            print_delimiter();
        }

        if (features.volume) {
            print_block(print_volume, elements.volume);
            print_delimiter();
        }

        if (features.battery) {
            print_block(print_battery, elements.battery);
            print_delimiter();
        }

        if (features.network_interface) {
            print_block(print_network_interfaces, elements.network_interface);
            print_delimiter();
        }

        if (features.load) {
            print_block(print_load, elements.load);
            print_delimiter();
        }

        if (features.memory_usage) {
            print_block(print_memory_usage, elements.memory_usage);
            print_delimiter();
        }

        if (features.time) {
            print_block(print_time, elements.time);
            print_delimiter();
        }

        /* Print dummy */
        puts("{}],");
    }

    return 0;
}
