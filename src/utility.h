#ifndef  __swaystatus_utility_H__
# define __swaystatus_utility_H__

# include <stddef.h>
# include <inttypes.h>
# include <sys/types.h>

# define MAX(arg1, arg2)     \
{                            \
    typeof(arg1) x = (arg1); \
    typeof(arg2) x = (arg2); \
    x > y ? x : y;           \
}

void* malloc_checked(size_t size);

void reallocarray_checked(void **ptr, size_t nmemb, size_t size);

/**
 * @param ptr must be l-value
 */
#define reallocate(ptr, n)                         \
    reallocarray_checked((void**) &(ptr), (n), sizeof(*ptr))

char* strdup_checked(const char *s);

/**
 * NOTE that msleep does not restart on interruption.
 */
void msleep(uintmax_t msec);

/**
 * @param dir should end with '/' or ""
 * @param dirfd if dir == "", then dirfd got to be AT_FDCWD.
 */
int openat_checked(const char *dir, int dirfd, const char *path, int flags);

ssize_t read_autorestart(int fd, void *buf, size_t count);

/**
 * @param len should be euqal to (file_size + 1), should be smaller than SSIZE_MAX.
 *            If the line is larger than len, then errx will be called.
 * @return if equal to len, then the file is bigger than expected.
 */
ssize_t readall(int fd, void *buffer, size_t len);

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

#endif
