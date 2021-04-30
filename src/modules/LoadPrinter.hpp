#ifndef  __swaystatus_LoadPrinter_H__
# define __swaystatus_LoadPrinter_H__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeLoadPrinter(void *config);
} /* namespace swaystatus::modules */

#endif
