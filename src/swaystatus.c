#include <stdio.h>
#include <unistd.h>

#include "print_time.h"

int main(int argc, char* argv[])
{
    setbuf(stdout, NULL);

    const char * const format = argc == 1 ? "%Y-%m-%d %T" : argv[1];

    for ( ; ; sleep(1))
        print_time(format);

    return 0;
}
