#include <cassert>

#include "mem_size_t.hpp"

using formatter = fmt::formatter<swaystatus::mem_size_t>;

auto formatter::parse(format_parse_context &ctx) -> format_parse_context_it
{
    auto it = ctx.begin(), end = ctx.end();
    if (it == end)
        return it;

    switch (*it) {
        default:
            FMT_THROW(format_error("invalid format"));

        case 'A':
        case 'K':
        case 'M':
        case 'G':
        case 'T':
        case 'P':
        case 'E':
        case 'Z':
        case 'Y':
            presentation = *it;
            /* If no terminating '}' is present */
            if (++it == end)
                FMT_THROW(format_error("invalid format"));
    }

    if (*it == '}')
        return it;

    if (*it != ' ')
        FMT_THROW(format_error("invalid format"));
    ++it;

    ctx.advance_to(it);
    return fmt::formatter<size_t>::parse(ctx);
}

static const char* get_unit(size_t ratio)
{
    switch (ratio) {
        case 1:
            return "B";
        case 2:
            return "K";
        case 3:
            return "M";
        case 4:
            return "G";
        case 5:
            return "T";
        case 6:
            return "P";
        case 7:
            return "E";
        case 8:
            return "Z";
        case 9:
            return "Y";

        default:
            return "Invalid unit";
    }
}
auto formatter::format_impl(size_t mem, const char *unit, format_context &ctx) -> format_context_it
{
    auto it = fmt::formatter<size_t>::format(mem, ctx);
    return fmt::format_to(it, "{}", unit);
}
auto formatter::auto_format(size_t mem, format_context &ctx) -> format_context_it
{
    size_t ratio = 1;
    for (; ratio < 9 && mem > 1024; ratio += 1)
        mem /= 1024;

    return format_impl(mem, get_unit(ratio), ctx);
}
auto formatter::format(const mem_size_t &sz, format_context &ctx) -> format_context_it
{
    size_t mem = sz.bytes;

    switch (presentation) {
        case 'B':
            return format_impl(mem, "B", ctx);

        case 'K':
            return format_impl(mem / 1024, "K", ctx);

        case 'M':
            return format_impl(mem / 1024 / 1024, "M", ctx);

        case 'G':
            return format_impl(mem / 1024 / 1024 / 1024, "G", ctx);

        case 'T':
            return format_impl(mem / 1024 / 1024 / 1024 / 1024, "T", ctx);

        case 'P':
            return format_impl(mem / 1024 / 1024 / 1024 / 1024 / 1024, "P", ctx);

        case 'E':
            return format_impl(mem / 1024 / 1024 / 1024 / 1024 / 1024 / 1024, "E", ctx);

        case 'Z':
            return format_impl(mem / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024, "Z", ctx);

        case 'Y':
            return format_impl(mem / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024 / 1024, "Y", ctx);

        case 'A':
            return auto_format(mem, ctx);

        default:
            assert(false);
    }
}
