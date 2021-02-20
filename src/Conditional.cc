#include <algorithm>

#include "Conditional.hpp"

using formatter = fmt::formatter<swaystatus::Conditional>;

template <class It>
static It find_end_of_format(It beg, It end)
{
    size_t level = 1;

    for (It it = beg; it != end; ++it) {
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

auto formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    /*
     * This function assumes that [it, end) points to the format passed from swaystatus::vprint
     */
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    end = find_end_of_format(it, end);

    if_true_str = std::string_view{it, static_cast<size_t>(end - it)};

    return end;
}

auto formatter::format(const Conditional &cond, format_context &ctx) -> format_context_it
{
    if (cond)
        return vformat_to(ctx.out(), if_true_str, ctx.args());
    else
        return ctx.out();
}
