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
#include "mem_size_t.hpp"
#include "print_memory_usage.h"

static int meminfo_fd;
static char *buffer;
static size_t buffer_sz;
static size_t memtotal;

extern "C" {
static void read_meminfo();
static size_t get_memusage(const char *element, size_t element_sz);

void init_memory_usage_collection()
{
    meminfo_fd = openat_checked("", AT_FDCWD, "/proc/meminfo", O_RDONLY);

    read_meminfo();
    memtotal = get_memusage("MemTotal:", sizeof("Memtotal:") - 1);
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

void print_memory_usage()
{
    read_meminfo();
    size_t memfree = get_memusage("MemFree:", sizeof("MemFree:") - 1);

    swaystatus::print("Mem Free={}/Total={:A}",
                      swaystatus::mem_size_t{memfree}, swaystatus::mem_size_t{memtotal});
}
}
