#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "print_memory_usage.h"

static int meminfo_fd;
static char *buffer;
static size_t buffer_sz;

void init_memory_usage_collection()
{
    meminfo_fd = openat_checked("", AT_FDCWD, "/proc/meminfo", O_RDONLY);
    buffer = malloc(100);
    if (!buffer)
        err(1, "%s failed", "malloc");
    buffer_sz = 100;
}

static size_t read_meminfo(void *buf, size_t len)
{
    ssize_t cnt = read_autorestart(meminfo_fd, buf, len);
    if (cnt < 0)
        err(1, "%s on %s failed", "read", "/proc/meminfo");
    return cnt;
}
static char* skip_space(char *str)
{
    while (*str == ' ')
        ++str;
    return str;
}

/**
 * @return free memory in byte
 */
static size_t get_memfree()
{
    char *line = NULL;
    char *eol;

    for (size_t cnt = 0; ;) {
        cnt += read_meminfo(buffer + cnt, buffer_sz - cnt - 1);
        buffer[cnt] = '\0';

        if (!line)
            line = strstr(buffer, "MemFree:");
        if (line) {
            eol = strstr(line, "\n");
            if (eol)
                break;
        }

        if (cnt == buffer_sz - 1)
            reallocate(buffer, (buffer_sz += 100));
    }

    if (lseek(meminfo_fd, 0, SEEK_SET) == (off_t) -1)
        err(1, "%s on %s failed", "lseek", "/proc/meminfo");

    line += sizeof("MemFree:") - 1;
    line = skip_space(line);

    errno = 0;
    char *endptr;
    uintmax_t val = strtoumax(line, &endptr, 10);
    if (errno == ERANGE)
        err(1, "%s on %s failed", "strtoumax", "/proc/meminfo");
    if (strncmp(endptr, " kB", 3) != 0)
        errx(1, "Unexpected format in %s", "/proc/meminfo");

    return val * 1000;
}

static const char *get_unit(size_t ratio)
{
    switch (ratio) {
        case 1:
            return "bytes";
        case 2:
            return "K";
        case 3:
            return "M";
        case 4:
            return "G";
        case 5:
            return "T";
        case 6:
            return "P";
        case 7:
            return "E";
        case 8:
            return "Z";
        case 9:
            return "Y";

        default:
            errx(1, "ratio %zu too large in %s:%d", ratio, __FILE__, __LINE__);
    }
}

void print_memory_usage()
{
    size_t memfree = get_memfree();

    size_t ratio = 1;
    for (; ratio < 9 && memfree > 1024; ratio += 1)
        memfree /= 1024;

    printf("MemFree: %zu %s", memfree, get_unit(ratio));
}
