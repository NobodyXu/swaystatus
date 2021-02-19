#ifndef  __swaystatus_printer_HPP__
# define __swaystatus_printer_HPP__

# include "dep/fmt/include/fmt/core.h"

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
    vprint(format, fmt::make_args_checked<Args...>(format, args...));
}

/**
 * Flush the buffer of stdout, not thread safe.
 */
void flush();

#endif
