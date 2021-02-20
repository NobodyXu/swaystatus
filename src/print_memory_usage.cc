#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "utility.h"
#include "printer.hpp"
#include "print_memory_usage.h"

struct ReadableMem {
    size_t mem;
    const char *unit;
};

static int meminfo_fd;
static char *buffer;
static size_t buffer_sz;
static struct ReadableMem memtotal;

extern "C" {

static struct ReadableMem get_readable_memusage(const char *element, size_t element_sz);
static void read_meminfo();

void init_memory_usage_collection()
{
    meminfo_fd = openat_checked("", AT_FDCWD, "/proc/meminfo", O_RDONLY);

    read_meminfo();
    memtotal = get_readable_memusage("MemTotal:", sizeof("Memtotal:") - 1);
}

static void read_meminfo()
{
    asreadall(meminfo_fd, &buffer, &buffer_sz);
    if (lseek(meminfo_fd, 0, SEEK_SET) == (off_t) -1)
        err(1, "%s on %s failed", "lseek", "/proc/meminfo");
}
static char* skip_space(char *str)
{
    while (*str == ' ')
        ++str;
    return str;
}

/**
 * @pre must call read_meminfo() before calling get_memusage.
 * @param element_sz excluding terminating null byte
 * @return in byte
 */
static size_t get_memusage(const char *element, size_t element_sz)
{
    char *line = strstr(buffer, element);
    if (!line)
        errx(1, "%s on %s failed", "Assumption", "/proc/meminfo");

    line += element_sz;
    line = skip_space(line);

    errno = 0;
    char *endptr;
    uintmax_t val = strtoumax(line, &endptr, 10);
    if (errno == ERANGE)
        err(1, "%s on %s failed", "strtoumax", "/proc/meminfo");
    if (strncmp(endptr, " kB\n", 4) != 0)
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
    read_meminfo();
    struct ReadableMem memfree = get_readable_memusage("MemFree:", sizeof("MemFree:") - 1);

    swaystatus::print("Mem {}={}{}/{}={}{}",
                      "Free", memfree.mem, memfree.unit, "Total", memtotal.mem, memtotal.unit);
}
}
