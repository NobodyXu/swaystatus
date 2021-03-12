#ifndef  __swaystatus_utility_H__
# define __swaystatus_utility_H__

# include <stddef.h>
# include <inttypes.h>
# include <sys/types.h>

# ifdef __cplusplus
extern "C" {
# endif

uintmax_t min_unsigned(uintmax_t x, uintmax_t y);

void* malloc_checked(size_t size);

void reallocarray_checked(void **ptr, size_t nmemb, size_t size);

/**
 * @param ptr must be l-value
 */
#define reallocate(ptr, n)                         \
    reallocarray_checked((void**) &(ptr), (n), sizeof(*ptr))

char* strdup_checked(const char *s);

/**
 * @return heap allocated string
 */
char* escape_quotation_marks(const char *fmt);

char* realpath_checked(const char *path);

void setenv_checked(const char *name, const char *value, int overwrite);

/**
 * NOTE that msleep does not restart on interruption.
 */
void msleep(uintmax_t msec);

void set_terminate_handler(void (*handler)());

void sigaction_checked_impl(int sig, const char *signame, void (*sighandler)(int signum));

# define sigaction_checked(sig, sighandler) sigaction_checked_impl((sig), # sig, (sighandler))

/**
 * close all fd except for 0, 1 and 2.
 */
void close_all();

/**
 * @param dir should end with '/' or ""
 * @param dirfd if dir == "", then dirfd got to be AT_FDCWD.
 * @param flags all fd will be opend with O_CLOEXEC.
 */
int openat_checked(const char *dir, int dirfd, const char *path, int flags);

void set_fd_non_blocking(int fd);

ssize_t read_autorestart(int fd, void *buf, size_t count);
ssize_t write_autorestart(int fd, const void *buf, size_t count);

/**
 * @return If equal to len, then the file is bigger than expected.
 *         -1 if read failed, error code is stored in errno
 */
ssize_t readall(int fd, void *buffer, size_t len);
/**
 * @param buffer it must point to a heap allocated buffer, can be NULL
 * @param len must point to length of *buffer, can be 0
 * @return -1 if read failed, error code is stored in errno.
 *
 * If buffer isn't large enough, then asreadall will realloc it
 * The read in buffer will be zero-terminated.
 */
ssize_t asreadall(int fd, char **buffer, size_t *len);

/**
 * @return function name/assumption that failed or NULL if succeeded.
 */
const char* readall_as_uintmax(int fd, uintmax_t *val);

/**
 * @param dir should end with '/'
 * @param path should be relative, eitherwise the error message will be confusing.
 *
 * isdir follows symlink
 */
int isdir(const char *dir, int dirfd, const char *path);

# ifdef __cplusplus
}

#  include <string>
#  include <initializer_list>
#  include <type_traits>

namespace swaystatus {
std::string getcwd_checked();

constexpr bool is_all_true(std::initializer_list<bool> list)
{
    for (bool each: list) {
        if (!each)
            return false;
    }
    return true;
}

template <class T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
} /* namespace swaystatus */
# endif

#endif
