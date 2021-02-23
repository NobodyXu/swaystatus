#ifndef  __swaystatus_fmt_utility_HPP__
# define __swaystatus_fmt_utility_HPP__

# include "fmt_config.h"
# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
using fmt::format_parse_context;
using format_parse_context_it = typename format_parse_context::iterator;

/**
 * Usage:
 *     auto it = ctx.begin(), end = ctx.end();
 *     if (it == end)
 *         return it;
 *     
 *     end = find_end_of_format(ctx);
 */
format_parse_context_it find_end_of_format(format_parse_context &ctx);
} /* namespace swaystatus */

#endif
