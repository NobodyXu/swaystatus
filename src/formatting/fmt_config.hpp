#include "../error_handling.hpp"

#include <exception>

#ifndef  FMT_HEADER_ONLY
# define FMT_HEADER_ONLY
#endif

#ifndef  FMT_USE_FLOAT
# define FMT_USE_FLOAT 0
#endif

#ifndef  FMT_USE_DOUBLE
# define FMT_USE_DOUBLE 0
#endif

#ifndef  FMT_USE_LONG_DOUBLE
# define FMT_USE_LONG_DOUBLE 0
#endif

# ifndef  FMT_STATIC_THOUSANDS_SEPARATOR
#  define FMT_STATIC_THOUSANDS_SEPARATOR ','
# endif

#ifndef  __swaystatus_fmt_config_HPP__
# define __swaystatus_fmt_config_HPP__

namespace swaystatus {
[[noreturn]]
void fmt_throw_impl(const std::exception &e, const char *func, int line, const char *file) noexcept;
} /* namespace swaystatus */

#endif

// Check if exceptions are disabled, copied from fmt
#ifndef CXX_HAS_EXCEPTION
# define FMT_THROW(x) ::swaystatus::fmt_throw_impl((x), __PRETTY_FUNCTION__, __LINE__, __FILE__)
#endif
