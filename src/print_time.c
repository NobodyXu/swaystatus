#define _POSIX_C_SOURCE 200112L /* For localtime_r */

#include <err.h>
#include <errno.h>

#include <time.h>
#include <unistd.h>

#include "process_configuration.h"
#include "printer.hpp"
#include "print_time.h"

static const char *user_specified_properties_str;

static const char *full_text_format;
static const char *short_text_format;

void init_time(void *config)
{
    full_text_format = get_format(config, "%Y-%m-%d %T");
    short_text_format = get_short_format(config, NULL);

    user_specified_properties_str = get_user_specified_property_str_impl(config, 0);
}

static void print_fmt(const struct tm *local_time, const char *name, const char *format)
{
    errno = 0;

    /*
     * allocate a big enough buffer to make sure strftime never fails
     */
    char buffer[4096];
    size_t cnt = strftime(buffer, sizeof(buffer), format, local_time);
    if (cnt == 0) {
        if (errno != 0)
            err(1, "strftime failed");
        else
            errx(1, "strftime returns 0: Your format string generate string longer than 4096 "
                    "which is larger than the buffer");
    }

    print_literal_str("\"");
    print_str(name);
    print_literal_str("\":\"");

    print_str2(buffer, cnt);

    print_literal_str("\",");
}
void print_time()
{
    const time_t epoch = time(NULL);
    struct tm local_time;
    /*
     * time technically can't fail as long as the first arg is set to NULL
     */
    const struct tm * const success = localtime_r(&epoch, &local_time);

    if (success == NULL)
        errx(1, "localtime_r failed due to time(NULL) has failed");

    print_fmt(&local_time, "full_text", full_text_format);
    if (short_text_format)
        print_fmt(&local_time, "short_text", short_text_format);

    print_str(user_specified_properties_str);
}
