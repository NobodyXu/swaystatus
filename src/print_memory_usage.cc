#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <stdlib.h>
#include <string.h>

#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <string_view>

#include "utility.h"
#include "printer.hpp"
#include "mem_size_t.hpp"
#include "LazyEval.hpp"
#include "print_memory_usage.h"

using namespace std::literals;
using swaystatus::mem_size_t;

static int meminfo_fd;
static char *buffer;
static size_t buffer_sz;
static size_t memtotal;

extern "C" {
static void read_meminfo();
static size_t get_memusage(std::string_view element);

void init_memory_usage_collection()
{
    meminfo_fd = openat_checked("", AT_FDCWD, "/proc/meminfo", O_RDONLY);

    read_meminfo();
    memtotal = get_memusage("MemTotal:"sv);
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
static size_t get_memusage(std::string_view element)
{
    char *line = strstr(buffer, element.data());
    if (!line)
        errx(1, "%s on %s failed", "Assumption", "/proc/meminfo");

    line += element.size();
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

static auto get_memusage_lazy(std::string_view element)
{
    return swaystatus::LazyEval{[=]() noexcept {
        return mem_size_t{get_memusage(element)};
    }};
}

void print_memory_usage()
{
    read_meminfo();

    swaystatus::print(
        "Mem Free={}/Total={}",
        get_memusage_lazy("MemFree:"sv),
        mem_size_t{memtotal}
    );
}
}
