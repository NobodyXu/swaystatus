#ifndef  __swaystatus_printer_HPP__
# define __swaystatus_printer_HPP__

# ifdef __cplusplus
#  include "dep/fmt/include/fmt/core.h"
# endif

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @param str must not be NULL
 */
void print_str(const char *str);

# ifdef __cplusplus
}
# endif

# ifdef __cplusplus
namespace swaystatus {
/**
 * @param str must not be nullptr
 */
inline void print_str(const char *str)
{
    ::print_str(str);
}

/**
 * Prints to the buffer of stdout, but does not flush the buffer, not thread safe.
 */
void vprint(fmt::string_view format, fmt::format_args args);

/**
 * Prints to the buffer of stdout, but does not flush the buffer, not thread safe.
 */
template <typename S, typename ...Args>
void print(const S &format, Args &&...args)
{
    swaystatus::vprint(format, fmt::make_args_checked<Args...>(format, args...));
}

/**
 * Flush the buffer of stdout, not thread safe.
 */
void flush();
} /* End of namespace swaystatus */
# endif

#endif
