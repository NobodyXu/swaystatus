#include <cstring>

#include <limits.h> /* For SSIZE_MAX and realpath */
#include <unistd.h>

#include <err.h>
#include <errno.h>

#include <exception>

#include "utility.h"

extern "C" {
void set_terminate_handler(void (*handler)())
{
    std::set_terminate(handler);
}
}

namespace swaystatus {
std::string getcwd_checked()
{
    std::string cwd;

    char *result;
    do {
        cwd.resize(cwd.size() + 200);
    } while ((result = getcwd(cwd.data(), cwd.size())) == NULL && errno == ERANGE);

    if (result == nullptr)
        err(1, "%s failed", "getcwd");

    cwd.resize(std::strlen(cwd.data()));

    return cwd;
}

ssize_t asreadall(int fd, std::string &buffer)
{
    if (buffer.size() < buffer.capacity())
        buffer.resize(buffer.capacity());

    size_t bytes = 0;
    for (ssize_t ret; ; bytes += ret) {
        if (buffer.size() == bytes)
            buffer.resize(buffer.size() * 2 + 100);

        auto *ptr = buffer.data();
        auto size = buffer.size();

        ret = read_autorestart(fd, ptr + bytes, min_unsigned(size - bytes, SSIZE_MAX));
        if (ret == 0)
            break;
        if (ret == -1)
            return -1;
    }

    buffer.resize(bytes);

    return bytes;
}
} /* namespace swaystatus */
