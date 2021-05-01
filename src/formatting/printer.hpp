#ifndef  __swaystatus_printer_HPP__
# define __swaystatus_printer_HPP__

# include <stddef.h>

# ifdef __cplusplus
#  include "fmt_config.hpp"
#  include "fmt/include/fmt/core.h"
# endif

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @param str must not be NULL
 */
void print_str(const char *str);
/**
 * @param str must not be NULL
 */
void print_str2(const char *str, size_t len);
/**
 * Flush the buffer of stdout, not thread safe.
 */
void flush();

# ifdef __cplusplus
}
# endif

# ifdef __cplusplus
#  define print_literal_str(literal) swaystatus::print_str2((literal), sizeof(literal) - 1)
# else
#  define print_literal_str(literal) print_str2((literal), sizeof(literal) - 1)
# endif

# ifdef __cplusplus
#  include <string_view>

namespace swaystatus {
/**
 * @param str must not be nullptr
 */
inline void print_str(const char *str)
{
    ::print_str(str);
}
/**
 * @param str must not be nullptr
 */
inline void print_str2(const char *str, size_t len)
{
    ::print_str2(str, len);
}

inline void print_str2(std::string_view sv)
{
    ::print_str2(sv.data(), sv.size());
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
inline void flush()
{
    ::flush();
}
} /* End of namespace swaystatus */
# endif

#endif
