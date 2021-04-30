#ifndef  __swaystatus_mem_size_t_H__
# define __swaystatus_mem_size_t_H__

# include "formatting/fmt_config.hpp"

# include <stddef.h>
# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
struct mem_size_t {
    size_t bytes;
};
}

/**
 * Expected replacement field for mem_size_t: "{A|K|M|G|T|P|E|Z|Y} {integer format}"
 */
template <>
struct fmt::formatter<swaystatus::mem_size_t>: fmt::formatter<size_t> {
    using format_parse_context = fmt::format_parse_context;
    using format_context = fmt::format_context;
    using mem_size_t = swaystatus::mem_size_t;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    char presentation = 'A';

    auto format_impl(size_t mem, const char *unit, format_context &ctx) -> format_context_it;
    auto auto_format(size_t mem, format_context &ctx) -> format_context_it;

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const mem_size_t &sz, format_context &ctx) -> format_context_it;
};

#endif
