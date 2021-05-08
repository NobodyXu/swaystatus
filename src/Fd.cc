#include <unistd.h>    /* For close */

#include "Fd.hpp"

namespace swaystatus {
Fd::Fd(int fd_arg) noexcept:
    fd{fd_arg}
{}

Fd::Fd(Fd &&other) noexcept:
    fd{other.fd}
{
    other.fd = -1;
}

Fd& Fd::operator = (Fd &&other) noexcept
{
    destroy();
    fd = other.fd;
    other.fd = -1;

    return *this;
}

void Fd::destroy() noexcept
{
    if (*this)
        close(fd);
}

Fd::~Fd()
{
    destroy();
}

Fd::operator bool () const noexcept
{
    return fd != -1;
}
int Fd::get() const noexcept
{
    return fd;
}
} /* namespace swaystatus */
