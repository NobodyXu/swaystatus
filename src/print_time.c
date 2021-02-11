#define _POSIX_C_SOURCE 200112L /* For localtime_r */

#include "print_time.h"

#include <stdio.h>
#include <err.h>

#include <time.h>
#include <unistd.h>

void print_time(const char * const format)
{
    const time_t epoch = time(NULL);
    struct tm local_time;
    /*
     * time technically can't fail as long as the first arg is set to NULL
     */
    const struct tm * const success = localtime_r(&epoch, &local_time);

    if (success == NULL)
        errx(1, "localtime_r failed due to time(NULL) has failed");

    /*
     * allocate a big enough buffer to make sure strftime never fails
     */
    char buffer[4096];
    size_t cnt = strftime(buffer, sizeof(buffer), format, &local_time);
    if (cnt == 0)
        errx(1, "strftime returns 0: buffer too small!");

    puts(buffer);
}
