#define _GNU_SOURCE     /* For RTLD_DEFAULT */
#define _DEFAULT_SOURCE /* For nice */
#define _POSIX_C_SOURCE /* For sigaction */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include <malloc.h>

#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

#include <execinfo.h>
#include <err.h>
#include <errno.h>

#include "help.h"
#include "utility.h"
#include "printer.hpp"
#include "process_configuration.h"

#include "poller.h"
#include "print_battery.h"
#include "print_time.h"
#include "print_volume.h"
#include "print_network_interfaces.h"
#include "print_brightness.h"
#include "print_memory_usage.h"
#include "print_load.h"

#define starts_with(str, prefix) (strncmp((str), (prefix), sizeof(prefix) - 1) == 0)

static void *buffer[20];
static void sigabort_handler(int sig)
{
    int sz = backtrace(buffer, sizeof(buffer) / sizeof(void*));
    backtrace_symbols_fd(buffer, sz, 2);
}

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
            fputs(help, stderr);
            exit(1);
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

    init_poller();

    if (features->time)
        init_time(get_format(config, "time", "%Y-%m-%d %T"));
    if (features->battery)
        init_battery_monitor(get_format(config, "battery", "{status} {capacity}%"));
    if (features->volume)
        init_volume_monitor(
            get_format(config, "volume", "vol {volume}%"),
            get_property(config, "volume", "mix_name", "Master"),
            get_property(config, "volume", "card",     "default")
        );
    if (features->network_interface)
        init_network_interfaces_scanning(
            get_format(
                config,
                "network_interface",
                "{is_connected:{per_interface_fmt_str:"
                    "{name} {is_dhcp:DHCP }in: {rx_bytes} out: {tx_bytes} "
                    "{ipv4_addrs:1} {ipv6_addrs:1}"
                "}}"
            ),
            60 * 10
        );
    if (features->brightness)
        init_brightness_detection(
            get_format(config, "brightness", "{backlight_device}: {brightness}")
        );
    if (features->memory_usage)
        init_memory_usage_collection(
            get_format(config, "memory_usage", "Mem Free={MemFree}/Total={MemTotal}")
        );
    if (features->load)
        init_load(
            get_format(config, "load", "1m: {loadavg_1m} 5m: {loadavg_5m} 15m: {loadavg_15m}")
        );

    config2json_elements_strs(config, elements);

    free_config(config);

    return interval;
}

static void print_block(void (*print)(), const char *json_element_str)
{
    print_literal_str("{\"full_text\":\"");
    print();
    print_literal_str("\",");
    print_str(json_element_str);
    print_literal_str("}");
}
static void print_delimiter()
{
    print_literal_str(",");
}

int main(int argc, char* argv[])
{
    /* Force dynamic linker to load function backtrace */
    if (dlsym(RTLD_DEFAULT, "backtrace") == NULL)
        err(1, "%s on %s failed", "dlsym", "backtrace");

    nice(19);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigabort_handler;
    if (sigaction(SIGABRT, &act, NULL) == -1)
        err(1, "%s on %s failed", "sigaction", "SIGABRT");

    struct Features features;
    struct JSON_elements_strs elements;
    const uintmax_t interval = parse_cmdline_arg_and_initialize(argc, argv, &features, &elements);

    /* Print header */
    print_literal_str("{\"version\":1}\n");
    flush();

    /* Begin an infinite array */
    print_literal_str("[\n");
    flush();

    for (size_t sec = 0; ; msleep(interval), ++sec) {
        perform_polling(0);

        print_literal_str("[");

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
        print_literal_str("{}],\n");
        flush();

        if (sec == 3660) {
            malloc_trim(4096 * 3);
            sec = 0;
        }
    }

    return 0;
}
