#include <dlfcn.h>

#include <err.h>

#include "utility.h"
#include "dynlib.hpp"

#include <array>
#include <memory>

std::array<std::string, 2> dlpaths;

extern "C" {
void setup_dlpath(const char *path)
{
    dlpaths[0] = swaystatus::getcwd_checked();
    dlpaths[0].shrink_to_fit();

    if (path)
        dlpaths[1] = path;
}

void* dload_symbol(const char *filename, const char *symbol_name)
{
    /* Clear any previous error */
    dlerror();

    void *handle = dlopen(filename, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
    if (handle == nullptr) {
        std::string path;

        for (const auto &dlpath: dlpaths) {
            if (dlpath.size() == 0)
                continue;

            path = dlpath + '/' + filename;
            handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
            path.clear();

            if (handle)
                break;
        }
        if (handle == nullptr)
            errx(1, "%s on %s failed: %s", "dload_symbol", filename, dlerror());
    }

     void *sym = dlsym(handle, symbol_name);
     const char *error = dlerror();
     if (sym == nullptr) {
        if (error)
            errx(1, "%s on %s failed: %s", "dlsym", symbol_name, error);
        else
            errx(
                1,
                "%s on %s failed: %s", "dlsym", symbol_name,
                "It is an undefined weak symboll/it is placed at address 0/IFUNC returns NULL"
            );
     }

     return sym;
}
} /* extern "C" */
