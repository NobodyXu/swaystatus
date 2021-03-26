#ifndef  __swaystatus_dylib_HPP__
# define __swaystatus_dylib_HPP__

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @param path can be NULL
 *
 * setup_dlpath should be called before chdir is called.
 */
void setup_dlpath(const char *path);

void* dload_symbol(const char *filename, const char *symbol_name);

# ifdef __cplusplus
} /* extern "C" */

# include <type_traits>

namespace swaystatus {
namespace impl {
template <class T>
struct ref2ptr {
    using type = T;

    static T cast(T val) noexcept
    {
        return val;
    }
};
template <class T>
struct ref2ptr<T&> {
    using type = T*;

    static T* cast(T &val) noexcept
    {
        return &val;
    }
};
template <class T>
struct ref2ptr<T&&> {
    using type = T*;

    static_assert(!std::is_fundamental_v<std::remove_cv_t<T>>);

    static T* cast(T &val) noexcept
    {
        return &val;
    }
};

template <class T>
using ref2ptr_t = typename ref2ptr<T>::type;
} /* namespace impl */

/**
 * class CFunction would automatically translate C++ function param to C param
 *
 * It will translate all reference to pointer, but it would not translate class
 * Thus all Args must be fundamental types or aggregates.
 */
template <class Ret, class ...Args>
class CFunction {
    using Fp = Ret (*)(impl::ref2ptr_t<Args>...);

    Fp ptr;
    
public:
    CFunction(const char *filename, const char *symbol_name):
        ptr{reinterpret_cast<Fp>(dload_symbol(filename, symbol_name))}
    {}

    CFunction(void *symbol):
        ptr{reinterpret_cast<Fp>(symbol)}
    {}

    CFunction(const CFunction&) = default;
    CFunction& operator = (const CFunction&) = default;

    decltype(auto) operator () (Args ...args)
    {
        return ptr(impl::ref2ptr<Args>::cast(args)...);
    }
};

} /* namespace swaystatus */
# endif

#endif
