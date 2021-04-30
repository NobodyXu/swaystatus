#define _GNU_SOURCE     /* For RTLD_DEFAULT */
#define _DEFAULT_SOURCE /* For nice */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include <malloc.h> /* For malloc_trim */

#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

#include <err.h>
#include <errno.h>

#include "help.h"
#include "utility.h"
#include "printer.hpp"
#include "python3.hpp"
#include "process_configuration.h"
#include "poller.h"

#define starts_with(str, prefix) (strncmp((str), (prefix), sizeof(prefix) - 1) == 0)

static void terminate_handler()
{
    stack_bt();
    _exit(1);
}
static void sigabort_handler(int sig)
{
    stack_bt();
}

static bool reload_requested;
static void handle_reload_request(int sig)
{
    reload_requested = true;
}

static uintmax_t parse_cmdline_arg_and_initialize(
    int argc, char* argv[],
    bool *is_reload, bool *is_click_event_enabled, const char **config_filename,
    struct Blocks *blocks
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
        } else if (strcmp(argv[i], "--reload") == 0) {
            *is_reload = true;
        } else {
            if (config)
                errx(1, "Error: configuration file is specified twice");
            *config_filename = realpath_checked(argv[i]);
            config = load_config(argv[i]);
        }
    }

    init_poller();

#ifdef USE_PYTHON
    if (!(*is_reload)) {
        if (*config_filename) {
            char *file = strdup_checked(*config_filename);
            char *path = dirname(file);

            setup_pythonpath(path);

            free(file);
        } else
            setup_pythonpath(NULL);
    }
#endif

    struct Inits inits;
    parse_inits_config(config, &inits);

    for (size_t i = 0; inits.inits[i]; ++i)
        inits.inits[i](get_module_config(config, inits.order[i]));

    *is_click_event_enabled = init_click_event_handlers(config, inits.order, *is_reload);

    get_block_printers(inits.order, blocks);

    free_config(config);

    return interval;
}

static void print_blocks(int fd, enum Event events, void *data)
{
    static const uintmax_t trim_interval = 3660;
    /**
     * trim heap right after first loop is done to give back memory
     * used during initialization time.
     */
    static uintmax_t cycle_cnt = trim_interval - 1;

    struct Blocks *blocks = data;

    print_literal_str("[");

    for (size_t i = 0; blocks->full_text_printers[i]; ++i) {
        blocks->full_text_printers[i]();
    }

    /* Print dummy */
    print_literal_str("{}],\n");
    flush();

    read_timer(fd);

    if (++cycle_cnt == trim_interval) {
        malloc_trim(4096 * 3);
        cycle_cnt = 0;
    }
}

int main(int argc, char* argv[])
{
    close_all();

    /* Force dynamic linker to load function backtrace */
    if (dlsym(RTLD_DEFAULT, "backtrace") == NULL)
        err(1, "%s on %s failed", "dlsym", "backtrace");

    nice(19);

    set_terminate_handler(terminate_handler);
    sigaction_checked(SIGABRT, sigabort_handler);
    sigaction_checked(SIGUSR1, handle_reload_request);

    struct Blocks blocks;

    bool is_reload = false;
    bool is_click_event_enabled;
    const char *config_filename = NULL;

    const uintmax_t interval = parse_cmdline_arg_and_initialize(
        argc, argv,
        &is_reload, &is_click_event_enabled, &config_filename,
        &blocks
    );

    if (chdir("/") < 0)
        err(1, "%s failed", "chdir(\"/\")");

    if (!is_reload) {
        /* Print header */
        print_literal_str("{\"version\":1");

        if (is_click_event_enabled)
            print_literal_str(",\"click_events\":true");

        print_literal_str("}\n");

        flush();

        /* Begin an infinite array */
        print_literal_str("[\n");
        flush();
    }

    int timerfd = create_pollable_monotonic_timer(interval);
    request_polling(timerfd, read_ready, print_blocks, &blocks);

    do {
        perform_polling(-1);
    } while (!reload_requested);

    if (reload_requested) {
        char buffer[4096];
        if (snprintf(buffer, sizeof(buffer), "--interval=%" PRIuMAX, interval) < 0)
            err(1, "%s on %s failed", "snprintf", "char buffer[4096]");

        execl("/proc/self/exe", "/proc/self/exe", buffer, "--reload", config_filename, NULL);
        err(1, "%s on %s failed", "execv", "/proc/self/exe");
    }

    return 0;
}
