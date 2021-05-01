#ifndef  __swaystatus_TimePrinter_HPP__
# define __swaystatus_TimePrinter_HPP__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeTimePrinter(void *config);
} /* namespace swaystatus::modules */

#endif
