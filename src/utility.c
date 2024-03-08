#define _DEFAULT_SOURCE /* For reallocarray, realpath and struct dirent::d_type */
#define _POSIX_C_SOURCE 200809L /* For openat, fstatat, sigaction */
#define _XOPEN_SOURCE 500 /* For realpath */

#include <stdio.h>
#include <limits.h> /* For SSIZE_MAX and realpath */
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <execinfo.h>

#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h> /* For timefd_create and timefd_settime */
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "utility.h"

uintmax_t min_unsigned(uintmax_t x, uintmax_t y)
{
    return x < y ? x : y;
}

void* malloc_checked(size_t size)
{
    void *ret = malloc(size);
    if (!ret)
        err(1, "%s failed", "malloc");
    return ret;
}
void reallocarray_checked(void **ptr, size_t nmemb, size_t size)
{
    void *newp = reallocarray(*ptr, nmemb, size);
    if (!newp)
        err(1, "%s failed", "reallocarray");
    *ptr = newp;
}

char* strdup_checked(const char *s)
{
    char *ret = strdup(s);
    if (!ret)
        err(1, "%s failed", "strdup");
    return ret;
}

char* escape_quotation_marks(const char *fmt)
{
    size_t fmt_len = strlen(fmt);

    /**
     * In the worst case scenario, every character is '"', thus the space required
     * is fmt_len * 2.
     */
    char *santilized = malloc(fmt_len * 2 * sizeof(char) + 1);

    size_t out = 0;
    for (size_t i = 0; i != fmt_len; ++i) {
        if (fmt[i] == '\"') {
            santilized[out++] = '\\';
            santilized[out++] = '\"';
        } else {
            santilized[out++] = fmt[i];
        }
    }
    santilized[out++] = '\0';
    reallocate(santilized, out);

    return santilized;
}

char* realpath_checked(const char *path)
{
    char *ret = realpath(path, NULL);
    if (ret == NULL)
        err(1, "%s on %s failed", "realpath", path);
    return ret;
}

void setenv_checked(const char *name, const char *value, int overwrite)
{
    if (setenv(name, value, overwrite) < 0)
        err(1, "%s failed", "setenv");
}

void msleep(uintmax_t msec)
{
    if (usleep(msec * 1000) && errno == EINVAL)
        err(1, "%s failed", "usleep");
}

int create_pollable_monotonic_timer(uintmax_t msec)
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd == -1)
        err(1, "%s failed", "timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)");

    struct itimerspec spec = {
        .it_interval = {
            .tv_sec = msec / 1000,
            .tv_nsec = (msec % 1000) * 1000 * 1000
        },
        /**
         * In order to arm the timer, either it_value.tv_sec or it_value.tv_nsec has to be non-zero
         * However, I'd like the initial expire of the timer to be ASAP, so tv_nsec is set to 
         * 1 while tv_sec is set to 0.
         */
        .it_value = {
            .tv_sec = 0,
            .tv_nsec = 1
        }
    };
    int result = timerfd_settime(timerfd, 0, &spec, NULL);
    if (result == -1)
        err(1, "%s failed", "timerfd_settime");

    return timerfd;
}
uint64_t read_timer(int timerfd)
{
    uint64_t ret;

    ssize_t bytes = read_autorestart(timerfd, (char*) &ret, sizeof(ret));
    if (bytes == -1)
        err(1, "%s on %s failed", "read_autorestart", "timerfd");

    return ret;
}

void sigaction_checked_impl(int sig, const char *signame, void (*sighandler)(int signum))
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sighandler;
    if (sigaction(sig, &act, NULL) == -1)
        err(1, "%s on %s failed", "sigaction", signame);
}

void close_all()
{
    DIR *fds = opendir("/proc/self/fd");
    if (fds == NULL)
        err(1, "%s on %s failed", "opendir", "/proc/self/fd");

    const int dir_fd = dirfd(fds);
    if (dir_fd == -1)
        err(1, "%s on %s failed", "dirfd", "dir opened on /proc/self/fd");

    errno = 0;
    for (struct dirent *ent; (ent = readdir(fds)); errno = 0) {
        if (ent->d_name[0] == '.')
            continue;

        errno = 0;
        char *endptr;
        int fd = strtoul(ent->d_name, &endptr, 10);
        if (errno != 0 || *endptr != '\0')
            err(1, "%s on %s failed", "Assumption", "/proc/self/fd");

        if (fd > 2 && fd != dir_fd)
            close(fd);
    }
    if (errno != 0)
        err(1, "%s on %s failed", "readdir", "/proc/self/fd");

    closedir(fds);
}

int openat_checked(const char *dir, int dirfd, const char *path, int flags)
{
    flags |= O_CLOEXEC;

    int fd;
    do {
        fd = openat(dirfd, path, flags);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1)
        err(1, "openat %s%s with %d failed", dir, path, flags);

    return fd;
}

void set_fd_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        err(1, "%s on %d failed", "fcntl(F_GETFL)", fd);

    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) < 0)
        err(1, "%s on %d failed", "Using fcntl to add O_NONBLOCK", fd);
}

ssize_t read_autorestart(int fd, void *buf, size_t count)
{
    ssize_t ret;
    do {
        ret = read(fd, buf, count);
    } while (ret == -1 && errno == EINTR);

    return ret;
}
ssize_t write_autorestart(int fd, const void *buf, size_t count)
{
    ssize_t ret;
    do {
        ret = write(fd, buf, count);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

ssize_t readall(int fd, void *buffer, size_t len)
{
    size_t bytes = 0;
    for (ssize_t ret; bytes < len; bytes += ret) {
        ret = read_autorestart(fd, (char*) buffer + bytes, min_unsigned(len - bytes, SSIZE_MAX));
        if (ret == 0)
            break;
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                return -1;
        }
    }

    return bytes;
}
ssize_t asreadall(int fd, char **buffer, size_t *len)
{
    size_t bytes = 0;
    for (ssize_t ret; ; bytes += ret) {
        if (bytes == *len) {
            *len += 100;
            reallocarray_checked((void**) buffer, *len, sizeof(char));
        }

        ret = read_autorestart(fd, *buffer + bytes, min_unsigned(*len - bytes, SSIZE_MAX));
        if (ret == 0)
            break;
        if (ret == -1)
            return -1;
    }

    if (bytes == *len) {
        *len += 1;
        reallocarray_checked((void**) buffer, *len, sizeof(char));
    }

    (*buffer)[*len - 1] = '\0';

    return bytes;
}

const char* readall_as_uintmax(int fd, uintmax_t *val)
{
    /*
     * 20 bytes is enough for storing decimal string as large as (uint64_t) -1
     */
    char line[21];
    ssize_t sz = readall(fd, line, sizeof(line));
    if (sz == -1)
        return "read";
    if (sz == 0 || sz == sizeof(line) || line[sz - 1] != '\n') {
        errno = 0;
        return "readall_as_uintmax assumption";
    }
    line[sz - 1] = '\0';

    errno = 0;
    char *endptr;
    uintmax_t ret = strtoumax(line, &endptr, 10);
    if (*endptr != '\0') {
        errno = 0;
        return "readall_as_uintmax assumption";
    }
    if (errno == ERANGE)
        return "strtoumax";

    *val = ret;
    return NULL;
}

int isdir(const char *dir, int dirfd, const char *path)
{
    struct stat dir_stat_v;
    if (fstatat(dirfd, path, &dir_stat_v, 0) < 0)
        err(1, "%s failed on %s%s", "stat", dir, path);

    return S_ISDIR(dir_stat_v.st_mode);
}

static void *bt_buffer[20];
void stack_bt()
{
    fputs("\n\n", stderr);

    int sz = backtrace(bt_buffer, sizeof(bt_buffer) / sizeof(void*));
    backtrace_symbols_fd(bt_buffer, sz, 2);
}

void visit_all_subdirs(const char *path, subdir_visiter visiter, ...)
{
    DIR *dir = opendir(path);
    if (!dir)
        err(1, "%s on %s failed", "opendir", path);

    va_list ap;
    va_start(ap, visiter);

    const int path_fd = dirfd(dir);

    errno = 0;
    for (struct dirent *ent; (ent = readdir(dir)); errno = 0) {
        switch (ent->d_type) {
            case DT_UNKNOWN:
            case DT_LNK:
                // Check if it is a dir after resolution
                if (!isdir(path, path_fd, ent->d_name))
                    break;
		//[[fallthrough]];

            case DT_DIR:
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                    visiter(path_fd, ent->d_name, ap);

            default:
                break;
        }
    }
    if (errno != 0)
        err(1, "%s on %s failed", "readdir", path);

    va_end(ap);
    closedir(dir);
}
