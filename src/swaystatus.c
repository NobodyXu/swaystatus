#include <stdio.h>
#include <unistd.h>

#include "print_battery.h"
#include "print_time.h"

int main(int argc, char* argv[])
{
    const char * const format = argc == 1 ? "%Y-%m-%d %T" : argv[1];

    struct Print_Battery_Data data;
    init_print_battery(&data);

    for ( ; ; sleep(1)) {
        print_battery(&data);
        fputs(" | ", stdout);
        print_time(format);

        puts("");
        fflush(stdout);
    }

    return 0;
}
