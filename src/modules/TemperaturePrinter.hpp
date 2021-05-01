#ifndef  __swaystatus_TemperaturePrinter_HPP__
# define __swaystatus_TemperaturePrinter_HPP__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeTemperaturePrinter(void *config);
} /* namespace swaystatus::modules */

#endif
