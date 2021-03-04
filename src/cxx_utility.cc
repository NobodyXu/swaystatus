#include <exception>

#include "utility.h"

extern "C" {
void set_terminate_handler(void (*handler)())
{
    std::set_terminate(handler);
}
}
