#define _POSIX_C_SOURCE 200112L /* For localtime_r */

#include <err.h>
#include <errno.h>

#include <time.h>
#include <unistd.h>

#include "TimePrinter.hpp"

using namespace std::literals;

namespace swaystatus::modules {
class TimePrinter: public Base {
    struct tm local_time;

public:
    TimePrinter(void *config):
        Base{
            config, "TimePrinter"sv,
            1, "%Y-%m-%d %T", nullptr
        }
    {}

    void update()
    {
        /*
         * time technically can't fail as long as the first arg is set to nullptr
         */
        const time_t epoch = time(nullptr);
        struct tm local_time;
        if (localtime_r(&epoch, &local_time) == nullptr)
            errx(1, "%s failed %s", "localtime_r", "due to time(nullptr) has failed");
    }
    void do_print(const char *format)
    {
        errno = 0;

        /*
         * allocate a big enough buffer to make sure strftime never fails
         */
        char buffer[4096];
        size_t cnt = strftime(buffer, sizeof(buffer), format, &local_time);
        if (cnt == 0) {
            if (errno != 0)
                err(1, "strftime failed");
            else
                errx(1, "strftime returns 0: Your format string generate string longer than 4096 "
                        "which is larger than the buffer");
        }

        print_str2(buffer, cnt);
    }
};

std::unique_ptr<Base> makeTimePrinter(void *config)
{
    return std::make_unique<TimePrinter>(config);
}
} /* namespace swaystatus::modules */
