#ifndef  __swaystatus_error_handling_HPP__
# define __swaystatus_error_handling_HPP__

# ifdef __cplusplus
#  if (defined(__GNUC__) && defined(__EXCEPTIONS)) || FMT_MSC_VER && _HAS_EXCEPTIONS
#   define CXX_HAS_EXCEPTION
#  endif
# endif

# ifdef CXX_HAS_EXCEPTION
#  define TRY try 
/**
 * Usage:
 *    CATCH (const std::exception &e) {
 *        // do anything here
 *    };
 */
#  define CATCH catch
# else
#  define TRY
#  define CATCH [&]
# endif

#endif
