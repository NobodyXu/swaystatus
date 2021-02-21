#ifndef  __swaystatus_LazyEval_H__
# define __swaystatus_LazyEval_H__

# include "fmt_config.h"

# include <type_traits>
# include "dep/fmt/include/fmt/format.h"

namespace swaystatus {
template <class F>
struct LazyEval {
    static_assert(std::is_invocable_v<F>);

    using result_type = std::invoke_result_t<F>;

    F f;

    decltype(auto) evaluate() const noexcept
    {
        return f();
    }
};
template <class F>
LazyEval(F f) -> LazyEval<F>;
}

template <class F>
struct fmt::formatter<swaystatus::LazyEval<F>>:
    fmt::formatter<typename swaystatus::LazyEval<F>::result_type>
{
    using parent_type = fmt::formatter<typename swaystatus::LazyEval<F>::result_type>;

    using format_parse_context = fmt::format_parse_context;
    using format_context = fmt::format_context;
    using LazyEval = swaystatus::LazyEval<F>;

    using format_parse_context_it = typename format_parse_context::iterator;
    using format_context_it = typename format_context::iterator;

    auto format(const LazyEval &val, format_context &ctx) -> format_context_it
    {
        return parent_type::format(val.evaluate(), ctx);
    }
};

#endif
