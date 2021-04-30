#ifndef  __swaystatus_modules_Base_HPP__
# define __swaystatus_modules_Base_HPP__

# include <cstdint>
# include <memory>
# include <string_view>

# include "../formatting/printer.hpp"

namespace swaystatus::modules {
class Base {
    // instance variables
    const std::string_view module_name;

    const std::unique_ptr<const char[]> full_text_format;
    const std::unique_ptr<const char[]> short_text_format;

    std::uint32_t cycle_cnt = 0;
    const std::uint32_t interval;

    std::unique_ptr<const char[]> user_specified_properties_str;
    std::size_t user_specified_properties_str_len;

    // instance methods

    /**
     * @param name need to be null-terminated
     */
    void print_fmt(std::string_view name, const char *format);

protected:
    Base() = delete;

    Base(const Base&) = delete;
    Base(Base&&) = delete;

    Base& operator = (const Base&) = delete;
    Base& operator = (Base&&) = delete;

    /**
     * @param module_name_arg should be null-terminated
     * @param n number of variadic args
     * @param args should be properties to be removed before converting this module_config 
     *             the a string `"property0": val, ...`, suitable for printing a json directly
     *             Eg: "\"border_top\":2,\"borer_left\":3"
     *
     * This ctor invokes uses n and args to invoke get_user_specified_property_str_impl2.
     */
    Base(void *config, std::string_view module_name_arg,
         std::uint32_t default_interval,
         const char *default_full_format, const char *default_short_format,
         unsigned n, ...);

    virtual void update();
    virtual void do_print(const char *format);

public:
    void update_and_print();

    virtual ~Base();
};
} /* namespace swaystatus::modules */

#endif
