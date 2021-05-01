#ifndef  __swaystatus_modules_Base_HPP__
# define __swaystatus_modules_Base_HPP__

# include <cstdint>
# include <utility>
# include <type_traits>
# include <memory>
# include <string_view>
# include <vector>

# include "../formatting/printer.hpp"

namespace swaystatus::modules {
namespace impl {
template <class ...Args>
struct is_cstr: public std::true_type {};

template <class T, class ...Args>
struct is_cstr<T, Args...>
{
    constexpr bool operator () () const noexcept
    {
        /**
         * Use if constexpr to avoid instantiation if possible.
         */
        if constexpr(std::is_same_v<std::decay_t<T>, const char*>)
            return is_cstr<Args...>{}();
        else
            return false;
    }
};

template <class ...Args>
inline constexpr const bool is_cstr_v = is_cstr<Args...>{}();
} /* namespace impl */

class Base {
    // instance variables
    const std::string_view module_name;

    const std::unique_ptr<const char[]> full_text_format;
    const std::unique_ptr<const char[]> short_text_format;

    std::uint32_t cycle_cnt;
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

    /**
     * Convenient wrapper
     */
    template <class ...Args, class = std::enable_if_t< impl::is_cstr_v<Args...> >>
    Base(
        void *config, std::string_view module_name_arg,
        std::uint32_t default_interval,
        const char *default_full_format, const char *default_short_format,
        Args &&...args
    ):
        Base{
            config, module_name_arg,
            default_interval, default_full_format, default_short_format,
            sizeof...(args), std::forward<Args>(args)...
        }
    {}

    virtual void update() = 0;
    virtual void do_print(const char *format) = 0;

public:
    /**
     * The first call to update_and_print will always trigger update
     */
    void update_and_print();

    virtual ~Base();
};

auto makeModules(void *config) -> std::vector<std::unique_ptr<Base>>;
} /* namespace swaystatus::modules */

#endif
