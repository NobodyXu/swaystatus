#ifdef USE_PYTHON

# include <cstddef>
# include <cstdarg>
# include <cstdlib>
# include <memory>

# include <err.h>

# define PY_SSIZE_T_CLEAN
# include <Python.h>

# include "utility.h"
# include "python3.hpp"

/*
 * https://docs.python.org/3/extending/embedding.html
 * https://docs.python.org/3/c-api/import.html
 * https://stackoverflow.com/questions/37605612/pyimport-importmodule-possible-to-load-module-from-memory
 * https://stackoverflow.com/questions/51507196/python-ctypes-to-return-an-array-of-function-pointers
 * https://stackoverflow.com/questions/61367199/handling-embedded-python-interpreter-calls-with-gil-and-multi-threading
 * https://github.com/pybind/pybind11
 * https://stackoverflow.com/questions/10625584/embedding-python-in-multithreaded-c-application
 * https://docs.python.org/3/c-api/init.html#non-python-created-threads
 * https://stackoverflow.com/questions/29595222/multithreading-with-python-and-c-api
 */

extern "C" {
void setup_pythonpath(const char *path)
{
    auto pythonpath = swaystatus::getcwd_checked();

    if (path) {
        pythonpath += ':';
        pythonpath += path;
    }

    auto *old_pythonpath = getenv("PYTHONPATH");
    if (old_pythonpath) {
        pythonpath += ':';
        pythonpath += old_pythonpath;
    }

    setenv_checked("PYTHONPATH", pythonpath.c_str(), 1);
}
} /* extern "C" */

struct raw_mem_deleter {
    void operator () (void *p) const noexcept
    {
        PyMem_RawFree(p);
    }
};

struct wchar_cstr {
    std::unique_ptr<wchar_t, raw_mem_deleter> str;
    std::size_t len;
};

static auto Py_DecodeLocale_Checked(const char *s) -> wchar_cstr
{
    wchar_cstr ret;

    auto *p = Py_DecodeLocale(s, &ret.len);
    if (p == nullptr) {
        if (ret.len == static_cast<std::size_t>(-1))
            errx(1, "%s failed: %s", "Py_DecodeLocale", "Insufficient memory");
        else if (ret.len == static_cast<std::size_t>(-2))
            errx(1, "%s failed: %s",
                    "Py_DecodeLocale",
                    "Decoding error - probably bug in the C library libpython depends on");
        else
            errx(1, "%s failed: %s", "Py_DecodeLocale", "Unknown error");
    }

    ret.str.reset(p);

    /* Rely on copy-elision in C++17 instead of move semantics */
    return ret;
}

/**
 * Assume the thread holds the GIL
 */
static void Py_Err(const char *fmt, ...)
{
    if (PyErr_Occurred() == nullptr)
        errx(1, "Internal bug: calling Py_Err when no exception is raised");
    PyErr_PrintEx(0);

    std::va_list args;
    va_start(args, fmt);
    verrx(1, fmt, args);
    va_end(args);
}

namespace swaystatus::python {
Interpreter::Interpreter(void *p) noexcept:
    p{p}
{}

Interpreter::GIL_scoped::GIL_scoped(Interpreter *interpreter) noexcept:
    interpreter{interpreter}
{}
Interpreter::GIL_scoped::~GIL_scoped()
{
    if (interpreter)
        interpreter->release();
}
auto Interpreter::acquire() -> GIL_scoped
{
    if (p == nullptr)
        return GIL_scoped{nullptr};

    PyEval_RestoreThread(static_cast<PyThreadState*>(p));
    p = nullptr;
    return GIL_scoped{this};
}
void Interpreter::release()
{
    p = PyEval_SaveThread();
}

static void initialize_interpreter()
{
    auto empty_wstr = Py_DecodeLocale_Checked("");

    wchar_t* argv[] = {empty_wstr.str.get(), nullptr};
    PySys_SetArgvEx(1, argv, 0);

    Module sys("sys");

    sys.setattr("stdin", Object::get_none());
    sys.setattr("stdout", Object::get_none());

    sys.setattr("__stdin__", Object::get_none());
    sys.setattr("__stdout__", Object::get_none());
}
void MainInterpreter::load_libpython3()
{
    if (Py_IsInitialized())
        return;

    auto wstr = Py_DecodeLocale_Checked("python3");
    Py_SetProgramName(wstr.str.get());

    /*
     * Changed in version 3.7: Py_Initialize() now initializes the GIL.
     */
    Py_InitializeEx(0);
    initialize_interpreter();
    get().release();
}
auto MainInterpreter::get() noexcept -> Interpreter&
{
    static MainInterpreter interpreter{nullptr};

    return interpreter;
}
bool MainInterpreter::has_initialized() noexcept
{
    return Py_IsInitialized();
}

SubInterpreter::SubInterpreter():
    Interpreter{Py_NewInterpreter()}
{
    if (p == nullptr)
        Py_Err("%s failed", "Py_NewInterpreter");

    auto *saved = PyEval_SaveThread();

    {
        auto scope = acquire();
        initialize_interpreter();
    }

    PyEval_RestoreThread(saved);
}
SubInterpreter::SubInterpreter(SubInterpreter &&other):
    Interpreter{other.p}
{
    other.p = nullptr;
}

SubInterpreter::~SubInterpreter()
{
    if (p) {
        auto *saved = PyEval_SaveThread();

        PyEval_RestoreThread(static_cast<PyThreadState*>(p));
        /*
         * Py_EndInterpreter needs the sub interpreter to be loaded.
         * After this call, the current thread state will be set to nullptr.
         */
        Py_EndInterpreter(static_cast<PyThreadState*>(p));

        PyEval_RestoreThread(saved);
    }
}

static PyObject* getPyObject(Object &o)
{
    return static_cast<PyObject*>(o.get());
}
static PyObject* getPyObject(const Object &o)
{
    return static_cast<PyObject*>(const_cast<void*>(o.get()));
}

Object Object::get_none() noexcept
{
    return {Py_None};
}
Object Object::create_new_ref(void *obj)
{
    if (obj)
        Py_INCREF(obj);

    return {obj};
}
Object Object::create_new_ref(const Object &obj)
{
    return create_new_ref(obj.obj);
}

Object::Object(void *obj) noexcept:
    obj{obj}
{}
Object::Object(Object &&other) noexcept:
    obj{other.obj}
{
    other.obj = nullptr;
}
Object& Object::operator = (Object &&other) noexcept
{
    free();
    obj = other.obj;
    other.obj = nullptr;

    return *this;
}

auto Object::get() noexcept -> void*
{
    return obj;
}
auto Object::get() const noexcept -> const void*
{
    return obj;
}

bool Object::has_value() const noexcept
{
    return obj != nullptr;
}

bool Object::is_none() const noexcept
{
    return obj == Py_None;
}

Object Object::getattr(const char *name)
{
    auto *ret = PyObject_GetAttrString(getPyObject(*this), name);
    if (ret == nullptr)
        errx(1, "Attr %s is not found in object %p", name, get());

    return {ret};
}
void Object::setattr(const char *name, Object object)
{
    if (PyObject_SetAttrString(getPyObject(*this), name, getPyObject(object)) == -1)
        Py_Err("%s failed", "PyObject_SetAttrString");
}

void Object::free() noexcept
{
    if (obj && obj != Py_None)
        Py_DECREF(getPyObject(*this));
}
Object::~Object()
{
    free();
}

static PyObject* compile(const char *code, const char *pseudo_filename, Compiled::Type type)
{
    PyCompilerFlags flags{
        .cf_flags = 0,
        .cf_feature_version = PY_MINOR_VERSION
    };
    int start;
    switch (type) {
        case Compiled::Type::file_like:
            start = Py_file_input;
            break;

        case Compiled::Type::single_line:
            start = Py_single_input;
            break;

        case Compiled::Type::single_expression:
            start = Py_eval_input;
            break;
    }
    auto *obj = Py_CompileStringExFlags(code, pseudo_filename, start, &flags, 2);

    if (obj == nullptr)
        Py_Err("%s on %s failed", "Py_CompileStringExFlags", code);

    return obj;
}
Compiled::Compiled(const char *pseudo_filename, const char *code, Type type):
    Object{compile(code, pseudo_filename, type)}
{}

Module::Module(const char *module_name):
    Object{PyImport_ImportModule(module_name)}
{
    if (get() == nullptr)
        Py_Err("%s on %s failed", "PyImport_ImportModule", module_name);
}

Module::Module(const char *persudo_module_name, Compiled &compiled):
    Object{PyImport_ExecCodeModule(persudo_module_name, getPyObject(compiled))}
{
    if (get() == nullptr)
        Py_Err("%s on %s failed", "PyImport_ExecCodeModule", persudo_module_name);
}

auto Module::getname() noexcept -> const char*
{
    return PyModule_GetName(getPyObject(*this));
}

Int::Int(ssize_t val):
    Object{PyLong_FromSsize_t(val)}
{
    if (get() == nullptr)
        Py_Err("%s failed: %s", "PyLong_FromSsize_t", "Possibly out of memory");
}

Int::Int(std::size_t val):
    Object{PyLong_FromSize_t(val)}
{
    if (get() == nullptr)
        Py_Err("%s failed: %s", "PyLong_FromSize_t", "Possibly out of memory");
}

static Object&& check_for_int(Object &o)
{
    auto *pyobject = getPyObject(o);
    if (pyobject != nullptr && PyLong_Check(pyobject) == 0)
        errx(1, "Object is not %s", "integer");

    return static_cast<Object&&>(o);
}
Int::Int(Object &&object):
    Object{check_for_int(object)}
{}

bool Int::to_ssize_t(ssize_t *val) const noexcept
{
    auto result = PyLong_AsSsize_t(getPyObject(*this));
    if (result == -1 && PyErr_Occurred())
        return false;

    *val = result;
    return true;
}
bool Int::to_size_t(std::size_t *val) const noexcept
{
    auto result = PyLong_AsSize_t(getPyObject(*this));
    if (result == static_cast<std::size_t>(-1) && PyErr_Occurred())
        return false;

    *val = result;
    return true;
}

static auto* PyMemoryView_FromMemory_Checked(const std::string_view &view)
{
    auto *ret = PyMemoryView_FromMemory(const_cast<char*>(view.data()), view.size(), PyBUF_READ);
    if (ret == nullptr)
        Py_Err("%s failed", "PyMemoryView_FromMemory");
    return ret;
}
MemoryView::MemoryView(const std::string_view &view):
    Object{PyMemoryView_FromMemory_Checked(view)}
{}

static auto* PyUnicode_DecodeUTF8_Checked(const std::string_view &view)
{
    auto *ret = PyUnicode_DecodeUTF8(view.data(), view.size(), nullptr);
    if (ret == nullptr)
        Py_Err("%s failed", "PyUnicode_DecodeUTF8");
    return ret;
}
str::str(const std::string_view &view):
    Object{PyUnicode_DecodeUTF8_Checked(view)}
{}

static Object&& check_for_str(Object &o)
{
    auto *pyobject = getPyObject(o);
    if (pyobject != nullptr && PyUnicode_Check(pyobject) == 0)
        errx(1, "Object is not %s", "string");

    return static_cast<Object&&>(o);
}
str::str(Object &&object):
    Object{check_for_str(object)}
{}

auto str::get_view() const noexcept -> std::string_view
{
    Py_ssize_t size;
    auto *s = PyUnicode_AsUTF8AndSize(getPyObject(*this), &size);
    if (s == nullptr)
        Py_Err("%s failed", "PyUnicode_AsUTF8AndSize");

    return {s, static_cast<std::size_t>(size)};
}
str::operator std::string_view () const noexcept
{
    return get_view();
}

auto tuple::get_creator() -> creator_t
{
    static_assert(std::is_same_v<Py_ssize_t, ssize_t>);
    return reinterpret_cast<creator_t>(PyTuple_Pack);
}
void tuple::handle_error(const char *msg)
{
    Py_Err(msg);
}

static Object&& check_for_tuple(Object &o)
{
    auto *pyobject = getPyObject(o);
    if (pyobject != nullptr && PyTuple_Check(pyobject) == 0)
        errx(1, "Object is not %s", "tuple");

    return static_cast<Object&&>(o);
}
tuple::tuple(Object &&object):
    Object{check_for_tuple(object)}
{}

auto tuple::size() const noexcept -> std::size_t
{
    return static_cast<std::size_t>(PyTuple_GET_SIZE(getPyObject(*this)));
}
auto tuple::get_element(std::size_t i) -> Object
{
    auto *result = PyTuple_GetItem(getPyObject(*this), i);
    if (result == nullptr)
        Py_Err("Out of bound access to tuple");

    return Object::create_new_ref(result);
}
auto tuple::operator [] (std::size_t i) -> Object
{
    return Object::create_new_ref(PyTuple_GET_ITEM(getPyObject(*this), i));
}

static Object&& check_for_callable(Object &o)
{
    auto *pyobject = getPyObject(o);
    if (pyobject != nullptr && PyCallable_Check(pyobject) == 0)
        errx(1, "object is not %s", "callable");

    return static_cast<Object&&>(o);
}
Callable_base::Callable_base(Object &&o):
    Object{check_for_callable(o)}
{}

auto Callable_base::get_caller() -> caller_t
{
    return reinterpret_cast<caller_t>(PyObject_CallFunctionObjArgs);
}

void Callable_base::handle_error()
{
    Py_Err("Calling object %p failed", get());
}
} /* namespace swaystatus::python */

#endif /* USE_PYTHON */
