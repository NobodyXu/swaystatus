#ifndef  __swaystatus_VolumePrinter_HPP__
# define __swaystatus_VolumePrinter_HPP__

# include "Base.hpp"

namespace swaystatus::modules {
std::unique_ptr<Base> makeVolumePrinter(void *config);
} /* namespace swaystatus::modules */

#endif
