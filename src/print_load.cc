#define _DEFAULT_SOURCE         /* For strsep */
#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <string.h>
#include <err.h>

#include <unistd.h> /* For lseek */
#include <fcntl.h>  /* For AT_FDCWD and O_RDONLY */

#include "utility.h"
#include "printer.hpp"
#include "print_load.h"

static const char * const loadavg_path = "/proc/loadavg";

static int load_fd;

extern "C" {
void init_load()
{
    load_fd = openat_checked("", AT_FDCWD, loadavg_path, O_RDONLY);
}

/**
 * @pre str != NULL, statistics != NULL
 */
static void split(char *str, const char *statistics[5])
{
    size_t i = 0;

    do {
        statistics[i++] = strsep(&str, " ");
    } while (str);

    if (i != 5)
        errx(1, "%s on %s failed", "Assumption", loadavg_path);
}

void print_load()
{
    /*
     * 100-long buffer should be enough for /proc/loadavg
     */
    char buffer[100];

    ssize_t cnt = readall(load_fd, buffer, sizeof(buffer) - 1);
    if (cnt == -1)
        err(1, "%s on %s failed", "readall", loadavg_path);
    if (cnt == 0 || cnt == sizeof(buffer) - 1)
        errx(1, "%s on %s failed", "Assumption", loadavg_path);
    buffer[cnt] = '\0';

    const char* statistics[5];
    split(buffer, statistics);

    swaystatus::print("1m: {} 5m: {} 15m: {}", statistics[0], statistics[1], statistics[2]);

    if (lseek(load_fd, 0, SEEK_SET) == (off_t) -1)
        err(1, "%s on %s failed", "lseek", loadavg_path);
}
}
