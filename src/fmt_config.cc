#include <cstdio>
#include <cstdlib>

#include "utility.h"

#include "fmt_config.hpp"

namespace swaystatus {
[[noreturn]]
void fmt_throw_impl(const std::exception &e, const char *func, int line, const char *file) noexcept
{
    std::fprintf(stderr, "Error when formatting: %s in %s, %d, %s\n", e.what(), func, line, file);
    stack_bt();
    std::exit(1);
}
} /* namespace swaystatus */
