#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "../../../src/utility.h"
#include "../../../src/Callback/dynlib.hpp"

using namespace swaystatus;

int main()
{
    {
        char *path = realpath_checked("test_dir");
        setup_dlpath(path);
        std::free(path);
    }

    int val = 2;

    assert((CFunction<int>{"libtest_lib.so", "f1"}()) == 1);
    assert((CFunction<int, int&>{"libtest_lib.so", "f2"}(val)) == val);

    assert((CFunction<int>{"libtest_lib2.so", "f1"}()) == 1);
    assert((CFunction<int, int&>{"libtest_lib2.so", "f2"}(val)) == val);

    return 0;
}
