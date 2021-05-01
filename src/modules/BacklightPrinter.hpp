#ifndef  __swaystatus_BacklightPrinter_H__
# define __swaystatus_BacklightPrinter_H__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeBacklightPrinter(void *config);
} /* namespace swaystatus::modules */

#endif
