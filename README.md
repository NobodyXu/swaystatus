# swaystatus

A lightweight yet feature-rich status bar for i3bar or swaybar.

![screenshot]

It is written completely in C/C++ to make it as lightweight as possible and specifically, to avoid creating new processes every second as in bash script.

It uses libraries like `libupower-glib`, `libasound` and `libnm` to retrieve battery, volume and network information as opposed to using `upower`, `amixer` or `nmcli`.

For backlight, load, and meminfo, it reads directly from `/sys/class/backlight`, `/proc/loadavg` and `/proc/meminfo`.

On my x86-64 computer, it compiles to a single binary that is only `64K` large using `clang-11`.

## Runtime Dependency
 - libupower-glib.so.3
 - libasound.so.2
 - libnm.so.0
 - libjson-c.so.5 (the same json library as sway and swaybar)

## Build

Install `clang` (must have lto support), `lld`, `make` and `pkg-config`, then run

```
git clone https://github.com/NobodyXu/swaystatus
cd swaystatus/src
make -j $(nproc)
```

To install, run `sudo make install`, which by default will install a single binary `swaystatus` to `/usr/local/bin`.

## Usage

### swaybar usage:

```
swaystatus: Usage: swaystatus [options] configuration_filename

  --help                    Show help message and exit
  --interval=unsigned_msec  Specify update interval in milliseconds, must be an unsignerinteger.
```

### Config file format

    {
        "name": {
            "format": "Hello, {variable_name}",
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
            "separator_block_width": 9
        },
    }

All property present for "name" above are optional.
<br>For volume, you can also set property "mix_name" and "card".
<br>NOTE that property "format" is unsupported by "network_interface".

The following values are valid name:

 - brightness
 - volume
 - battery
 - network_interface
 - load
 - memory_usage
 - time

If you want to disable a certain feature, say brightness,
then add the following to your configuration:

    {
        "brightness": false,
    }

#### Battery format variables:

 - `state`
 - `level`
 - `temperature`
 - `is_fully_charged` (Check section "Conditional Variable" for usage)
 - `is_charging`
 - `is_discharging`
 - `is_empty`

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
 - `has_multiple_backlight_devices` (this is a Conditional Variable)

#### Network Interface variables;

 - `is_network_enabled`
 - `is_not_network_enabled`

 - `has_active_connection`
 - `has_no_active_connection`
 - `has_no_connection`
 - `has_connection`
 - `has_full_connection`
 - `has_limited_connection`
 - `has_portal_connection`
 
 - `connectivity_state`
 - `ipv4_config`
 - `ipv6_config`

`ipv4_config` and `ipv6_config` each denotes an array of valid ip addresses.

To limit number of ip addresses in output, please use `{ipv4_config:1}`.

#### Format string for time:

Format string for time is parsed by strftime instead of fmtlib, so the format is
specified in [`strftime`] instead.

For format string other than time, check [fmt - Format String Syntax] for more
information on format specification.Conditional Variable:

#### Conditional Variables

Conditional variables are used to selectively print strings.

For example, setting "format" in "battery" to "{is_charging:Charging}" will print "Charging" only
when the battery is charging.

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
