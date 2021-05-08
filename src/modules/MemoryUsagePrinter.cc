#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <unistd.h>
#include <fcntl.h>

#include <cstddef>
#include <err.h>

#include <string>

#include "../utility.h"
#include "../Fd.hpp"

#include "../formatting/printer.hpp"
#include "../formatting/LazyEval.hpp"
#include "../mem_size_t.hpp"
#include "MemoryUsagePrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class MemoryUsagePrinter: public Base {
    static constexpr const char * const path = "/proc/meminfo";

    Fd meminfo_fd;
    std::size_t memtotal = -1;
    std::string buffer;

    void read_meminfo()
    {
        ssize_t cnt = asreadall(meminfo_fd.get(), buffer);
        if (cnt < 0)
            err(1, "%s on %s failed", "read", path);
    
        if (lseek(meminfo_fd.get(), 0, SEEK_SET) == (off_t) -1)
            err(1, "%s on %s failed", "lseek", path);
    }
    char* skip_space(char *str)
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
    std::size_t get_memusage(std::string_view element)
    {
        char *line;
        do {
            line = strstr(buffer.data(), element.data());
            if (!line)
                return -1;
        } while (line[element.size()] != ':');

        line += element.size() + 1;
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

    auto get_memusage_lazy(std::string_view element)
    {
        return swaystatus::LazyEval{[=]() noexcept {
            return mem_size_t{get_memusage(element)};
        }};
    }

public:
    MemoryUsagePrinter(void *config):
        Base{
            config, "MemoryUsagePrinter"sv,
            10, "Mem Free={MemFree}/Total={MemTotal}", nullptr
        },
        meminfo_fd{openat_checked("", AT_FDCWD, path, O_RDONLY)}
    {}

    void update()
    {
        read_meminfo();
        if (UNLIKELY(memtotal == -1))
            memtotal = get_memusage("MemTotal"sv);
    }
    void do_print(const char *format)
    {
        print(
            format,
            fmt::arg("MemFree", get_memusage_lazy("MemFree")),
            fmt::arg("MemAvailable", get_memusage_lazy("MemAvailable")),
            fmt::arg("Buffers", get_memusage_lazy("Buffers")),
            fmt::arg("Cached", get_memusage_lazy("Cached")),
            fmt::arg("SwapCached", get_memusage_lazy("SwapCached")),
            fmt::arg("Active", get_memusage_lazy("Active")),
            fmt::arg("Inactive", get_memusage_lazy("Inactive")),
            fmt::arg("Mlocked", get_memusage_lazy("Mlocked")),
            fmt::arg("SwapTotal", get_memusage_lazy("SwapTotal")),
            fmt::arg("SwapFree", get_memusage_lazy("SwapFree")),
            fmt::arg("Dirty", get_memusage_lazy("Dirty")),
            fmt::arg("Writeback", get_memusage_lazy("Writeback")),
            fmt::arg("AnonPages", get_memusage_lazy("AnonPages")),
            fmt::arg("Mapped", get_memusage_lazy("Mapped")),
            fmt::arg("Shmem", get_memusage_lazy("Shmem")),
            fmt::arg("MemTotal", mem_size_t{memtotal})
        );
    }
    void reload()
    {}
};

std::unique_ptr<Base> makeMemoryUsagePrinter(void *config)
{
    return std::make_unique<MemoryUsagePrinter>(config);
}
} /* namespace swaystatus::modules */
