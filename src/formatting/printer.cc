#include "fmt_config.hpp"

#include <string.h>
#include <err.h>

#include <algorithm>

#include "../dep/fmt/include/fmt/format.h"

#include "../utility.h"
#include "printer.hpp"

static fmt::basic_memory_buffer<char, /* Inline buffer size */ 4096> out;

extern "C" {
void print_str(const char *str)
{
    /* len, excluding the terminating null byte */
    const size_t len = strlen(str);

    print_str2(str, len);
}
void print_str2(const char *str, size_t len)
{
    out.append(str, str + len);
}

void flush()
{
    const char *data = out.data();
    const size_t sz = out.size();

    for (size_t cnt = 0; cnt != sz; ) {
        ssize_t ret = write_autorestart(
            1,
            data + cnt,
            std::min(sz - cnt, static_cast<size_t>(SSIZE_MAX))
        );
        if (ret < 0)
            err(1, "%s on %s failed", "write", "fd 1");
        cnt += ret;
    }

    out.clear();
}
}

namespace swaystatus {
void vprint(fmt::string_view format, fmt::format_args args)
{
    fmt::vformat_to(out, format, args);
}
} /* End of namespace swaystatus */
