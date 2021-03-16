#include <cstdio>
#include <cstdlib>
#include <cassert>

#define USE_PYTHON
#include "../../../src/python3.hpp"
#include "../../../src/utility.h"

using namespace swaystatus::python;

int main()
{
    // Initialize libpython
    {
        char *path = realpath_checked("python3_test_dir");
        setup_pythonpath(path);

        MainInterpreter::load_libpython3();
        std::free(path);
    }

    std::printf("%s = %s\n\n", "PYTHONPATH", getenv("PYTHONPATH"));

    {
        auto scope = MainInterpreter::get().acquire();
        static_cast<void>(scope);

        Compiled compiled{
            "print_sys_path",

            "import sys\n"
            "def f():\n"
            "    return str(sys.path)\n"
            "\n"
        };
        Module print_sys_path{"print_sys_path", compiled};
        Callable<str> f{print_sys_path.getattr("f")};
        str sys_path{f()};
        std::printf("sys.path = %s\n\n", sys_path.get_view().data());

        Module hello_world{"hello_world"};
        Callable<str> hello{hello_world.getattr("hello")};

        assert(str{hello()}.get_view() == "Hello, world!");

        Module test{"test"};
        Callable<Int, std::size_t> identity{test.getattr("identity")};

        ssize_t val;
        assert(identity(2021).to_ssize_t(&val));

        assert(val == 2021);
    }

    ;

    return 0;
}
