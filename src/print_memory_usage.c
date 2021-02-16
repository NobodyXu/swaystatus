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

struct ReadableMem {
    size_t mem;
    const char *unit;
};

static int meminfo_fd;
static char *buffer;
static size_t buffer_sz;
static struct ReadableMem memtotal;

static struct ReadableMem get_readable_memusage(const char *element, size_t element_sz);

void init_memory_usage_collection()
{
    meminfo_fd = openat_checked("", AT_FDCWD, "/proc/meminfo", O_RDONLY);
    buffer = malloc(100);
    if (!buffer)
        err(1, "%s failed", "malloc");
    buffer_sz = 100;
    memtotal = get_readable_memusage("MemTotal:", sizeof("Memtotal:") - 1);
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
 * @param element_sz excluding terminating null byte
 * @return in byte
 */
static size_t get_memusage(const char *element, size_t element_sz)
{
    char *line = NULL;
    char *eol;

    for (size_t cnt = 0; ;) {
        cnt += read_meminfo(buffer + cnt, buffer_sz - cnt - 1);
        buffer[cnt] = '\0';

        if (!line)
            line = strstr(buffer, element);
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

    line += element_sz;
    line = skip_space(line);

    errno = 0;
    char *endptr;
    uintmax_t val = strtoumax(line, &endptr, 10);
    if (errno == ERANGE)
        err(1, "%s on %s failed", "strtoumax", "/proc/meminfo");
    if (strncmp(endptr, " kB", 3) != 0)
        errx(1, "%s on %s failed", "Assumption", "/proc/meminfo");

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
static struct ReadableMem get_readable_memusage(const char *element, size_t element_sz)
{
    size_t mem = get_memusage(element, element_sz);

    size_t ratio = 1;
    for (; ratio < 9 && mem > 1024; ratio += 1)
        mem /= 1024;

    return (struct ReadableMem){
        .mem = mem,
        .unit = get_unit(ratio)
    };
}

void print_memory_usage()
{
    struct ReadableMem memfree = get_readable_memusage("MemFree:", sizeof("MemFree:") - 1);

    printf("%s %zu %s/%s %zu %s",
           "MemFree:", memfree.mem, memfree.unit, "MemTotal:", memtotal.mem, memtotal.unit);
}
