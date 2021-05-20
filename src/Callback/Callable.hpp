#ifndef  __swaystatus_Callable_HPP__
# define __swaystatus_Callable_HPP__

# include <err.h>

# include <variant>

# include "../utility.h"
# include "python3.hpp"
# include "dynlib.hpp"

namespace swaystatus {
namespace impl {
template <class ...Args>
struct is_valid_callable_args_t {
    constexpr bool operator () () const noexcept
    {
        return true;
    }
};

template <class T, class ...Args>
struct is_valid_callable_args_t<T, Args...> {
    constexpr bool operator () () const noexcept
    {
        if constexpr(std::is_fundamental_v< rm_cvref_t<T> > && std::is_rvalue_reference_v<T>)
            return false;
        else
            return is_valid_callable_args_t<Args...>{}();
    }
};

template <class ...Args>
constexpr bool is_valid_callable_args() noexcept
{
    return is_valid_callable_args_t<Args...>{}();
}
} /* namespace impl */

class Callable_base {
    std::variant<
        std::monostate,
        /* pointer to the c function */
        void*
# ifdef USE_PYTHON
        , python::Callable_base
# endif
    > v;

    template <class Ret, class ...Args>
    friend class Callable;

public:
    Callable_base() = default;

    Callable_base(const char *name, const void *callable_config);

    Callable_base(Callable_base&&) = default;
    Callable_base& operator = (Callable_base&&) = default;

    ~Callable_base() = default;
};

template <class Ret, class ...Args>
class Callable {
    static_assert(impl::is_valid_callable_args<Args...>());

    struct monostate {
        Ret operator () (Args ...args) const noexcept
        {
            if constexpr(!std::is_void_v<Ret>)
                return Ret{};
        }
    };

# ifdef USE_PYTHON
    using py_callback = python::Callable<
        std::conditional_t<std::is_void_v<Ret>, python::None, Ret>, 
        Args...
    >;
# endif

    std::variant<
        monostate,
        CFunction<Ret, Args...>
# ifdef USE_PYTHON
        , py_callback
# endif
    > v;

public:
    Callable() = default;

    Callable(Callable_base &&base)
    {
# ifdef USE_PYTHON
        if (auto p = std::get_if<python::Callable_base>(&base.v); p) {
            v.template emplace<py_callback>(std::move(*p));
        } else
# endif
        if (auto p = std::get_if<void*>(&base.v); p) {
            v.template emplace<CFunction<Ret, Args...>>(*p);
        }
    }

    Callable(Callable&&) = default;
    Callable& operator = (Callable&&) = default;

    ~Callable() = default;

    auto operator () (Args ...args) -> Ret
    {
# ifdef USE_PYTHON
        auto scope = std::holds_alternative<py_callback>(v) ? 
            python::MainInterpreter::get().acquire() :
            python::Interpreter::GIL_scoped(nullptr);
# endif

        if constexpr(std::is_void_v<Ret>) {
            std::visit([&](auto &&f) {
                f(std::forward<Args>(args)...);
            }, v);
        } else {
            return std::visit([&](auto &&f)
            {
                return f(std::forward<Args>(args)...);
            }, v);
        }
    }
};
} /* namespace swaystatus */

#endif
