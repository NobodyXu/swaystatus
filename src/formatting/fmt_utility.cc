#include "fmt_utility.hpp"

namespace swaystatus {
format_parse_context_it find_end_of_format(format_parse_context &ctx)
{
    size_t level = 1;

    for (auto it = ctx.begin(); it != ctx.end(); ++it) {
        if (*it == '{')
            ++level;
        if (*it == '}')
            --level;
        if (level == 0)
            return it;
    }

    FMT_THROW(fmt::format_error("invalid format: Unterminated '{'"));
    __builtin_unreachable();
}
} /* namespace swaystatus */
