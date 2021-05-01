#ifndef  __swaystatus_NetworkInterfacesPrinter_HPP__
# define __swaystatus_NetworkInterfacesPrinter_HPP__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeNetworkInterfacesPrinter(void *config);
} /* namespace swaystatus::modules */

#endif
