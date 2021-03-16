#ifndef  __swaystatus_python3_HPP__
# define __swaystatus_python3_HPP__

# ifdef USE_PYTHON

#  ifdef __cplusplus
extern "C" {
#  endif

/**
 * @param path can be NULL
 *
 * setup_pythonpath should be called before chdir is called.
 */
void setup_pythonpath(const char *path);

#  ifdef __cplusplus
}

# include <cinttypes>
# include <utility>
# include <type_traits>
# include <initializer_list>

# include "utility.h"

namespace swaystatus::python {
class Interpreter {
protected:
    void *p;

    Interpreter(void *p) noexcept;

public:
    /**
     * GIL_scoped automatically call release() in dtor.
     */
    class GIL_scoped {
        Interpreter *interpreter;

    public:
        GIL_scoped(Interpreter *interpreter) noexcept;
        ~GIL_scoped();
    };
    /**
     * Interpreter::acquire() must be called before any method in this header other than
     * load_libpython3 can be invoked
     *
     * acquire can be called on the same object multiple times
     * and only the first call will actually acquire the GIL lock.
     */
    auto acquire() -> GIL_scoped;
    void release();
};

class MainInterpreter: public Interpreter {
    using Interpreter::Interpreter;

public:
    /**
     * @pre setup_pythonpath has been invoked
     *
     * load_libpython3() will only initialize libpython on the first call.
     */
    static void load_libpython3();

    static bool has_initialized() noexcept;

    static auto get() noexcept -> Interpreter&;
};

class SubInterpreter: public Interpreter {
public:
    SubInterpreter();

    SubInterpreter(const SubInterpreter&) = delete;
    SubInterpreter(SubInterpreter&&);

    SubInterpreter& operator = (const SubInterpreter&) = delete;
    SubInterpreter& operator = (SubInterpreter&&) = delete;

    /**
     * ~SubInterpreter() can only be called when the SubInterpreter is not loaded (after release()).
     */
    ~SubInterpreter();
};

class Object {
protected:
    void *obj;

    void free() noexcept;

public:
    /**
     * @return object denoting 'None'.
     */
    static Object get_none() noexcept;

    /**
     * @param obj create_new_ref will call Py_INCREF on obj.
     *            obj can be nullptr.
     */
    static Object create_new_ref(void *obj);
    /**
     * @param obj create_new_ref will call Py_INCREF on obj.get()
     */
    static Object create_new_ref(const Object &obj);

    /**
     * @param obj Object will not call Py_INCREF on obj
     */
    Object(void *obj = nullptr) noexcept;

    Object(const Object&) = delete;
    /**
     * @param object after move, no function can be called on object except
     *               the move assignment and destructor
     */
    Object(Object &&object) noexcept;

    Object& operator = (const Object&) = delete;
    /**
     * @param object after move, no function can be called on object except
     *               the move assignment and destructor
     */
    Object& operator = (Object&&) noexcept;

    auto get() noexcept -> void*;
    auto get() const noexcept -> const void*;

    /**
     * Test that this object actually contains a PyObject
     */
    bool has_value() const noexcept;

    bool is_none() const noexcept;

    Object getattr(const char *name);
    void   setattr(const char *name, Object object);

    ~Object();
};

template <class T>
static constexpr const bool is_object_v = std::is_base_of_v<Object, rm_cvref_t<T>>;

class tuple;
/**
 * Cusomtization point for converting C++ type to python type
 */
template <class T, class = void>
struct Conversion {
    using result_type = tuple;
};

template <class T>
struct Conversion<T, std::enable_if_t<std::is_base_of_v< Object, rm_cvref_t<T> >>> {
    using result_type = T;
};

template <class T>
using conversion_result_t = typename Conversion<T>::result_type;

class Compiled: public Object {
public:
    /**
     * The type of input
     */
    enum class Type {
        file_like,
        single_line,
        single_expression,
    };
    Compiled(const char *pseudo_filename, const char *code, Type type = Type::file_like);
};

class Module: public Object {
public:
    Module(const char *module_name);
    Module(const char *persudo_module_name, Compiled &compiled);

    auto getname() noexcept -> const char*;
};

class Int: public Object {
public:
    Int(ssize_t);
    Int(std::size_t);

    Int(Object &&object);

    /**
     * @param val (*val) is only changed on success.
     * @return true on success, false if overflow
     */
    bool to_ssize_t(ssize_t *val) const noexcept;
    bool to_size_t(std::size_t *val) const noexcept;

    /**
     * WARNING: implicit conversion does not check for overflow
     */
    template <class T, class = std::enable_if_t< std::is_integral_v<T> >>
    operator T () const noexcept
    {
        if constexpr(std::is_signed_v<T>) {
            ssize_t val;
            to_ssize_t(&val);
            return val;
        } else {
            std::size_t val;
            to_size_t(&val);
            return val;
        }
    }
};

template <class T>
struct Conversion<T, std::enable_if_t<std::is_constructible_v< Int, T >>> {
    using result_type = Int;
};

class MemoryView: public Object {
public:
    MemoryView(const std::string_view &view);
};

template <class T>
struct Conversion<T, std::enable_if_t<std::is_constructible_v< MemoryView, T >> > {
    using result_type = MemoryView;
};

class str: public Object {
public:
    str(const std::string_view &view);

    str(Object &&object);

    auto get_view() const noexcept -> std::string_view;
    operator std::string_view () const noexcept;
};

class tuple: public Object {
protected:
    using creator_t = void* (*)(ssize_t n, ...);

    static auto get_creator() -> creator_t;

    static void handle_error(const char *msg);

    template <class ...Args>
    static void* create_tuple_checked(Args &&...args)
    {
        auto *creator = get_creator();
        auto *ret = creator(
            sizeof ...(args),
            conversion_result_t<Args>(std::forward<Args>(args)).get()...
        );
        if (ret == nullptr)
            handle_error("Failed to create tuple");

        return ret;
    }

public:
    template <class ...Args>
    tuple(Args &&...args):
        Object{create_tuple_checked(std::forward<Args>(args)...)}
    {}

private:
    template <class Unpackable, std::size_t ...I>
    tuple(Unpackable &&unpackable, std::index_sequence<I...>):
        tuple{std::get<I>(std::forward<Unpackable>(unpackable))...}
    {}

public:
    /**
     * @param unpackable can be std::tuple, std::pair, std::array, or any type
     *                   that support std::get and std::tuple_size
     */
    template <
        class Unpackable,
        class = std::void_t<decltype(std::get<0>(std::declval<Unpackable>()))>,
        class = std::void_t<decltype(std::tuple_size<rm_cvref_t<Unpackable>>::value)>
    >
    tuple(Unpackable &&unpackable):
        tuple{
            std::forward<Unpackable>(unpackable),
            std::make_index_sequence<std::tuple_size<rm_cvref_t<Unpackable>>::value>{}
        }
    {}

    template <class T, class = std::void_t<decltype( to_unpackable(std::declval<T>()) )>>
    tuple(T &&obj):
        tuple{to_unpackable(std::forward<T>(obj))}
    {}

    tuple(Object &&object);

    /**
     * @pre has_value() == true
     */
    auto size() const noexcept -> std::size_t;
    /**
     * @pre has_value() == true
     * @param i if i > size(), call Py_Err.
     */
    auto get_element(std::size_t i) -> Object;
    /**
     * @pre has_value() == true
     * @param i argument isn't checked, it is UB if i > size().
     */
    auto operator [] (std::size_t i) -> Object;

private:
    template <class T, std::size_t ...I>
    T convert_to_impl(std::index_sequence<I...>)
    {
        return {std::tuple_element_t<I, T>{(*this)[I]}...};
    }

public:
    template <class T>
    T convert_to()
    {
        static_assert(!std::is_reference_v<T>);
        static_assert(!std::is_const_v<T>);
        static_assert(!std::is_volatile_v<T>);

        if (size() < std::tuple_size<T>::value)
            handle_error("Conversion from swaystatus::python::tuple failed");

        return convert_to_impl<T>(std::make_index_sequence<std::tuple_size<T>::value>{});
    }

    template <class T>
    operator T ()
    {
        return convert_to<T>();
    }
};

class Callable_base: public Object {
protected:
    using caller_t = void* (*)(void*, ...);

    static auto get_caller() -> caller_t;

    void handle_error();

public:
    /**
     * Convert Object to Callable
     *
     * Call errx if the object is not a callable or its signature does not fit.
     */
    explicit Callable_base(Object &&o);

    /**
     * @exception If the object throws, then the error is printed to stderr and _exit is called.
     */
    template <
        class ...Args,
        class = std::enable_if_t<
            is_all_true({!std::is_const_v<Args>...}) && is_all_true({is_object_v<Args>...})
        >
    >
    Object operator () (Args &&...args)
    {
        auto *ret = (get_caller())(get(), std::forward<Args>(args).get()..., NULL);
        if (ret == nullptr)
            handle_error();

        return Object{ret};
    }

    Object operator () ()
    {
        auto *ret = (get_caller())(get(), NULL);
        if (ret == nullptr)
            handle_error();

        return Object{ret};
    }
};

template <class Ret, class ...Args>
class Callable: public Callable_base {
public:
    /**
     * Convert Object to Callable
     *
     * Call errx if the object is not a callable or its signature does not fit.
     */
    explicit Callable(Object &&o):
        Callable_base{std::move(o)}
    {}

    Callable(Callable_base &&o):
        Callable_base{std::move(o)}
    {}

    Ret operator () (Args ...args)
    {
        auto &base = static_cast<Callable_base&>(*this);

        return Ret{base(static_cast<conversion_result_t<Args>>(std::forward<Args>(args))...)};
    }
};
} /* namespace swaystatus */
#  endif

# endif /* USE_PYTHON */

#endif
