#ifndef  __swaystatus_help_H__
# define __swaystatus_help_H__

# ifdef __cplusplus
extern "C" {
# endif

static const char * const help = 
    "Usage: swaystatus [options] configuration_filename\n\n"
    "  --help                    Show help message and exit\n"
    "  --interval=unsigned_msec  Specify update interval in milliseconds, must be an unsigner "
                                 "integer.\n"
    "                            By default, the interval is set to 1000 ms.\n\n";

# ifdef __cplusplus
}
# endif

#endif
