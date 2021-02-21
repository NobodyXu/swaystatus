/**
 * Conditional is an extension to fmt
 *
 * Format: "{format_arg_name:format_str}"
 *
 * Yes, its'right: Conditional supports recursive format_str.
 */

#ifndef  __swaystatus_conditional_H__
# define __swaystatus_conditional_H__

# include "fmt_config.h"

# include <type_traits>
# include <string_view>

# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
struct Conditional {
    bool value;

    constexpr operator bool () const noexcept
    {
        return value;
    }
};
}


template <>
struct fmt::formatter<swaystatus::Conditional>
{
    using Conditional = swaystatus::Conditional;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    std::string_view if_true_str = "";

    auto parse(format_parse_context &ctx) -> format_parse_context_it;
    auto format(const Conditional &cond, format_context &ctx) -> format_context_it;
};

#endif
