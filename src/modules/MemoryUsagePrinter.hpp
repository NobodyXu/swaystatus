#ifndef  __swaystatus_MemoryUsagePrinter_H__
# define __swaystatus_MemoryUsagePrinter_H__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeMemoryUsagePrinter(void *config);
} /* namespace swaystatus::modules */

#endif
