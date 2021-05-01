#ifndef  __swaystatus_BatteryPrinter_H__
# define __swaystatus_BatteryPrinter_H__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeBatteryPrinter(void *config);
} /* namespace swaystatus::modules */

#endif
