#include <exception>

#include <cstring>

#include <unistd.h>
#include <err.h>
#include <errno.h>

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

} /* namespace swaystatus */
