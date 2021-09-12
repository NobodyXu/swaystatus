#include <cstdio>
#include <cstdlib>

#include "../utility.h"

#include "fmt_config.hpp"

namespace swaystatus {
static const char *module = nullptr;
void fmt_set_calling_module(const char *module_arg) noexcept
{
    (void) module_arg;
#ifndef CXX_HAS_EXCEPTION
    module = module_arg;
#endif
}

[[noreturn]]
void fmt_throw_impl(const std::exception &e, const char *func, int line, const char *file) noexcept
{
    std::fprintf(stderr,
                 "Error when formatting in module %s: %s in %s, %d, %s\n",
                 module ? module : "nullptr", e.what(), func, line, file);
    stack_bt();
    std::exit(1);
}
} /* namespace swaystatus */
