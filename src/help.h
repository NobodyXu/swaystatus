#ifndef  __swaystatus_help_H__
# define __swaystatus_help_H__

# ifdef __cplusplus
extern "C" {
# endif

static const char * const help = 
    "Usage: swaystatus [options] configuration_filename\n\n"
    "  --help                    Show help message and exit\n"
    "  --interval=unsigned_msec  Specify update interval in milliseconds, must be an unsigner"
    "integer.\n\n"
    "Config file format:\n\n"
    "    {\n"
    "        \"name\": {\n"
    "            \"format\": \"##RRGGBBA\",\n"
    "            \"color\": \"##RRGGBBA\",\n"
    "            \"background: \"##RRGGBBA\",\n"
    "            \"border\": \"##RRGGBBA\",\n"
    "            \"border_top\": 1,\n"
    "            \"border_bottom\": 1,\n"
    "            \"border_left\": 1,\n"
    "            \"border_right\": 1,\n"
    "            \"min_width\": 1,\n"
    "            \"align\": \"center\",\n"
    "            \"separator\": true,\n"
    "            \"separator_block_width\": 9\n"
    "        },\n"
    "    }\n\n"
    "All property present for \"name\" above are optional.\n"
    "For volume, you can also set property mix_name and card.\n"
    "NOTE that property \"format\" is now only supported by time and battery.\n\n"
    "The following values are valid name:\n\n"
    " - brightness\n"
    " - volume\n"
    " - battery\n"
    " - network_interface\n"
    " - load\n"
    " - memory_usage\n"
    " - time\n\n"
    "If you want to disable a certain feature, say brightness,\n"
    "then add the following to your configuration:\n\n"
    "    {\n"
    "        \"brightness\": false,\n"
    "    }\n\n"
    "Battery format variables:\n\n"
    " - state\n"
    " - level\n"
    " - temperature\n"
    " - is_fully_charged (Check \"Conditional Variable\" section for usage)\n"
    " - is_charging\n"
    " - is_discharging\n"
    " - is_empty\n\n"
    "Conditional Variable:\n\n"
    "Conditional variables are used to selectively print strings.\b"
    "For example, setting \"format\" in \"battery\" to \"{is_charging:Charging} will print"
    "\"Charging\" only when the battery is charging.\n\n"
    "Recursive Conditional Variable:\n\n"
    "In additional to printing strings conditionally, conditional variables can also be used to\n"
    "print other variables conditionally.\n"
    "For example, \"{is_charging:{level}%}\" will print \"98%\" when charging, where \n"
    "\"98\" is the actual level of battery."
;

# ifdef __cplusplus
}
# endif

#endif
