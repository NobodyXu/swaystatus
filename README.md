# swaystatus

A lightweight yet feature-rich status bar for i3bar or swaybar.

![screenshot]

It is written completely in C/C++ to make it as lightweight as possible and specifically, to avoid creating new processes every second as in bash script.

It uses libraries like `libasound` and glibc function `getifaddrs` to retrieve volume information as opposed to using `amixer` and `ip addr`.

For battery, backlight, load, and meminfo, it reads directly from `/sys/class/power_supply`,
`/sys/class/backlight`, `/proc/loadavg` and `/proc/meminfo`.

## Runtime Dependency
 - `libasound.so.2`
 - `libjson-c.so.5` (also used by sway and swaybar)
 - `libsensors.so.5`

## Build

Install `clang` (must have lto support), `lld`, `make` and `pkg-config`, then run

```
git clone --recurse-submodules https://github.com/NobodyXu/swaystatus
cd swaystatus/src
make -j $(nproc)
```

You can customize ou build via environment variable:
 - `BUILD_DIR`: affects where the built object will be put. Default is `.`.
 - `TARGET_DIR`: where the executable will be installed when `make install` is executed. Default is `/usr/local/bin`
 - `PYTHON`: whether to include embeded python interpreter support in `swaystatus`, can be `true` or `false`. Default is `true`.
 - `DEBUG`: whether to have a debug build or release build. `true` for debug build and `false`for release build. Default is `false`.
 - `EXCEPTION`: whether to enable C++ exception. `false` to disable C++ exception.

To install, run `sudo make install`, which by default will install a single binary `swaystatus` to `/usr/local/bin`.

## Usage

### swaybar usage:

```
swaystatus: Usage: swaystatus [options] configuration_filename

  --help                    Show help message and exit
  --interval=unsigned_msec  Specify update interval in milliseconds, must be an unsigner integer.
                            By default, the interval is set to 1000 ms.
```

To reload `swaystatus`, send `SIGUSR1` to `swaystatus` process.

### Config file format

    {
        "order": ["network_interface", "time"],
        "_comment": "element order specify the order of which blocks will appear.",
        "_comment2": "If a block is not specified in order, it will not appear.",

        "name": {
            "format": "Hello, {variable_name}",
            "short_format": "hello", 
            "update_interval": 20,
            "color": "##RRGGBBA",
            "background: "##RRGGBBA",
            "border": "##RRGGBBA",
            "border_top": 1,
            "border_bottom": 1,
            "border_left": 1,
            "border_right": 1,
            "min_width": 1,
            "align": "center",
            "separator": true,
            "separator_block_width": 9,
            "markup": "none",

            "click_event_handler": {
                "type": "python",
                "module_name": "hello",
                "function_name": "handler"
            }
        },
        "_comment": "Any variable starts with '_' is a comment"
    }

All property present for "name" above are optional.
<br>For volume, you can also set property "mix_name" and "card".

The following values are valid name:

 - brightness
 - volume
 - battery

   The configuration block of battery support `excluded_model` to exclude certain battery devices
   from the output of `swaystatus`.
 - network_interface
 - load
 - memory_usage
 - time
 - sensors

**Any unrecognized parameters will be ignored.**

#### "format"

It is used internally to generate the "full_text" that will be passed to swaybar.

Formatting for blocks other than time is done using [fmt - Format String Syntax].
<br>Formatting for block time is parsed by [`strftime`].

In the format string, user is capable of taking advantage of 
[format variables](/#Format_Variables) to customize the output.

#### "short_format"

It is used to generate "short_text", which is used by `swaybar` when it decided that the 
"full_text" is too long.

#### Click Event Handling Support

TO enable click event handling for a block, add json object "click_event_handler" 
to the block that block.

The return value of it can be `or`ed from the following values:
 - `0` do nothing
 - `1` force the module to update
 - `2` force the module to reload (not yet implemented)

Currently, the handler can be written in python or C/C++.

##### Loading python handler

For loading python handler, add `"type": "python"` to your "click_event_handler", then specify 
the "module_name" and "function_name" of the handler.

`swaystatus` will attempt to load the module from the same directory of your configuration file, 
your current working dir, any path you specified with environment variable `PYTHONPATH` and your 
system module paths.

If instead you want to embed the python code into your configuration file, add "code" to your 
"click_event_handler" and assign your code as value to it.

The python function is expected to have signature (Check [here](https://www.mankier.com/7/swaybar-protocol#Click_Events) for more information):

```
(instance: str, click_pos: Tuple[int, int], button: int, event: int, relative_click_pos: Tuple[int, int], blocksize: Tuple[int, int]) -> int
```

Your function is expected to return 0 and any exception thrown in your function must be handled, 
otherwise `swaystatus` will print that error and exit.

##### Loading C/C++ handler

You C/C++ code has to be compiled with `-fPIC -shared`.

For loading python handler, add `"type": "dynlib"` to your "click_event_handler", then specify 
the "module_name" and "function_name" of the handler.

Just like loading python, `swaystatus` will attemp to load the shared library from the same 
directory of your configuration file, your current working dir.

It is loaded using `dlopen`, so `LD_LIBRARY_PATH`, `/etc/ld.so.cache`, `/lib` and `/usr/lib` is 
also searched.

You can also specify the path instead of a name directly in the `module_name`, which is perfectly 
valid.

The C/C++ function is expected to be `export`ed in `"C"` and have signature:

```c
#include <stdint.h>

struct Pos {
    uint64_t x;
    uint64_t y;
};
typedef struct Pos ClickPos;
typedef struct Pos BlockSize;

int f(
    const char *instance, 
    const ClickPos *pos,
    uint64_t button,
    uint64_t event,
    const ClickPos *relative_pos,
    const BlockSize *size
)
```

Your function is expected to return 0.

#### Disable block

If you want to disable a certain feature, say brightness,
then add the following to your configuration:

    {
        "brightness": false,
    }

Or if you are using element "order" for specifing block order, then you can simply
remove the name from "order".

Note that `{"brightness": false}` overrides "order":
<br>If block specified in "order" is disabled by setting it to `false`, then the block
will not appear.

#### `update_interval`

If specified, then the block will update at `update_interval * main_loop_interval ms`,
where `main_loop_interval` is the value passed by cmdline arg `--interval=` or `1000 ms`
by default.

##### `update_interval` for `sensors`

The sensors on computer are so many that they cannot be fitted into one line, so `swaystatus`
instead, print one sensor each time and once `update_interval` is reached, next sensor is shown
at the `swaybar`.

When all sensors are shown, `swaystatus` will then update the sensors reading from the computer.

### Format_Variables

#### Battery format variables:

 - `has_battery`: check whether battery device exists
 - `per_battery_fmt_str`, which is used to print every battery on this system.
   
   It contains the following variables:
    - `name`
    - `present`
    - `technology`
    - `model_name`
    - `manufacturer`
    - `serial_number`
    - `status`
    - `cycle_count`
    - `voltage_min_design`
    - `voltage_now`
    - `charge_full_design`
    - `charge_full`
    - `charge_now`
    - `capacity`
    - `capacity_level`
    - `is_charging`        (Check section "Conditional Variable" for usage)
    - `is_discharging`
    - `is_not_charging`
    - `is_full`

#### Memory Usage variables:

 - `MemTotal`
 - `MemFree`
 - `MemAvailable`
 - `Buffers`
 - `Cached`
 - `SwapCached`
 - `Active`
 - `Inactive`
 - `Mlocked`
 - `SwapTotal`
 - `SwapFree`
 - `Dirty`
 - `Writeback`
 - `AnonPages`
 - `Mapped`
 - `Shmem`

The unit (supports 'BKMGTPEZY') of the variables printed can be specified.
<br>For example, '{MemTotal:K}' will print MemTotal in KiloBytes.

#### Volume variables:

 - `volume`

#### Load variables:

 - `loadavg_1m`
 - `loadavg_5m`
 - `loadavg_15m`
 - `running_kthreads_cnt`
 - `total_kthreads_cnt`
 - `last_created_process_pid`

#### Brightness variables:

NOTE that these variables are evaluated per backlight_device.

 - `backlight_device`
 - `brightness`
 - `max_brightness`
 - `has_multiple_backlight_devices` (this is a Conditional Variable)

#### Network Interface variables;

 - `is_connected`
 - `is_not_connected`
 - `per_interface_fmt_str`:
    The specification for this variable will be formatted once for each interface.

    It contains subvariables that can be used inside:
     - `HAS_UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO`
     - `has_broadcast_support`
     - `is_pointopoint`
     - `has_no_arp_support`
     - `is_in_promisc_mode`
     - `is_in_notrailers_mode`
     - `is_master`
     - `is_slave`
     - `has_multicast_support`
     - `has_portsel_support`
     - `is_automedia_active`
     - `is_dhcp`
	 - `rx_packets`
	 - `tx_packets`
	 - `rx_bytes` (Supports unit specification, checks [Memry Usage Variables](#memory-usage-variables) for more info)
	 - `tx_bytes` (Supports unit specification, checks [Memry Usage Variables](#memory-usage-variables) for more info)
	 - `rx_errors`
	 - `tx_errors`
	 - `rx_dropped`
	 - `tx_dropped`
	 - `multicast`
	 - `collisions`
	 - `rx_length_errors`
	 - `rx_over_errors`
	 - `rx_crc_errors`
	 - `rx_frame_errors`
	 - `rx_fifo_errors`
	 - `rx_missed_errors`
	 - `tx_aborted_errors`
	 - `tx_carrier_errors`
	 - `tx_fifo_errors`
	 - `tx_heartbeat_errors`
	 - `tx_window_errors`
	 - `rx_compressed`
	 - `tx_compressed`
     - `ipv4_addrs`
     - `ipv6_addrs`

    Limit of number of ip address can be done via `{ipv4_addrs:1}` and `{ipv6_addrs:1}`

    Optionally if `HAS_UAPI_DEF_IF_NET_DEVICE_FLAGS_LOWER_UP_DORMANT_ECHO`, 
    the following variables are also defined:
     - `is_lower_up`
     - `is_dormant`
     - `is_echo_device`

To limit number of ip addresses in output, please use `{ipv4_config:1}`.

#### Sensors variables;

 - `prefix`: the name of the device
 - `path`: the path to the device in `/sys`
 - `addr`: the internal address of the device in `libsensors`
 - `bus_type`: the type of device
 - `bus_nr`: unclear
 - `reading_number`: the internal index for the specific sensor reading
 - `reading_temp`: the temperature reading from the sensor

#### Format string for time:

#### Conditional Variables

Conditional variables are used to conditionally evaulate part of the format string.

For example, setting "format" in "battery" to "{is_charging:Charging}" will print "Charging" only
when the battery is charging.

All variables start with "is" and "has" are conditional variables.

#### Recursive Conditional Variable:

In additional to printing strings conditionally, conditional variables can also be used to
print other variables conditionally.

For example, "{is_charging:{level}%}" will print "98%" when charging, where
"98" is the actual level of battery.

Check [`example-config.json`] for the example configuration.

### Use `swaybar` in `sway`

```
bar {
    status_command swaystatus
}
```

[screenshot]: https://raw.githubusercontent.com/NobodyXu/swaystatus/main/screenshot.png
[`strftime`]: https://man7.org/linux/man-pages/man3/strftime.3.html
[fmt - Format String Syntax]: https://fmt.dev/latest/syntax.html
[`example-config.json`]: https://github.com/NobodyXu/swaystatus/blob/main/example-config.json
