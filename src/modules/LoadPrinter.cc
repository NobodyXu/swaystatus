#define _DEFAULT_SOURCE         /* For strsep */
#define _POSIX_C_SOURCE 200809L /* For AT_FDCWD */

#include <err.h>

#include <unistd.h> /* For lseek */
#include <fcntl.h>  /* For AT_FDCWD and O_RDONLY */

#include "../utility.h"

#include "LoadPrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class LoadPrinter: public Base {
    int load_fd;
    /*
     * 100-long buffer should be enough for /proc/loadavg
     */
    char buffer[100];
    const char* statistics[6];

    /**
     * @pre str != NULL, statistics != NULL
     */
    void split(char *str, const char *statistics[5])
    {
        size_t i = 0;
    
        do {
            statistics[i++] = strsep(&str, " ");
        } while (str);
    
        if (i != 5)
            errx(1, "%s on %s failed", "Assumption", loadavg_path);
    }
    void parse_loadavg(char *str, const char *statistics[6])
    {
        char *delimiter = strchr(str, '/');
        if (!delimiter)
            errx(1, "%s on %s failed", "Assumption", loadavg_path);
    
        split(str, statistics);
    
        statistics[5] = statistics[4];
        statistics[4] = delimiter + 1;
        *delimiter = '\0';
    }

    void update_load()
    {
        ssize_t cnt = readall(load_fd, buffer, sizeof(buffer) - 1);
        if (cnt == -1)
            err(1, "%s on %s failed", "readall", loadavg_path);
        if (cnt == 0 || cnt == sizeof(buffer) - 1)
            errx(1, "%s on %s failed", "Assumption", loadavg_path);
        buffer[cnt] = '\0';
    
        if (lseek(load_fd, 0, SEEK_SET) == (off_t) -1)
            err(1, "%s on %s failed", "lseek", loadavg_path);
    
        parse_loadavg(buffer, statistics);
    }

public:
    static constexpr const char * const loadavg_path = "/proc/loadavg";

    LoadPrinter(void *config):
        Base{
            config, "LoadPrinter"sv,
            60, "1m: {loadavg_1m} 5m: {loadavg_5m} 15m: {loadavg_15m}", nullptr
        }
    {
        load_fd = openat_checked("", AT_FDCWD, loadavg_path, O_RDONLY);
        update_load();
    }

    void update()
    {
        update_load();
    }

    void do_print(const char *format)
    {
        print(
            format,
            fmt::arg("loadavg_1m", statistics[0]),
            fmt::arg("loadavg_5m", statistics[1]),
            fmt::arg("loadavg_15m", statistics[2]),
            fmt::arg("running_kthreads_cnt", statistics[3]),
            fmt::arg("total_kthreads_cnt", statistics[4]),
            fmt::arg("last_created_process_pid", statistics[5])
        );
    }
};

std::unique_ptr<Base> makeLoadPrinter(void *config)
{
    return std::make_unique<LoadPrinter>(config);
}
} /* namespace swaystatus::modules */
