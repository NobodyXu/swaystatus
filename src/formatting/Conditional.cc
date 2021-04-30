#include <algorithm>

#include "fmt_utility.hpp"
#include "Conditional.hpp"

using formatter = fmt::formatter<swaystatus::Conditional>;

using swaystatus::find_end_of_format;

auto formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    /*
     * This function assumes that [it, end) points to the format passed from swaystatus::vprint
     */
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    end = find_end_of_format(ctx);

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
