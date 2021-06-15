#ifndef  __swaystatus_CustomPrinter_HPP__
# define __swaystatus_CustomPrinter_HPP__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeCustomPrinter(void *config);
} /* namespace swaystatus::modules */

#endif
