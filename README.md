# swaystatus

A lightweight yet feature-rich status bar for i3bar or swaybar.

![screenshot]

It is written completely in C/C++ to make it as lightweight as possible and specifically, to avoid creating new processes every second as in bash script.

It uses libraries like `libupower-glib`, `libasound` and `libnm` to retrieve battery, volume and network information as opposed to using `upower`, `amixer` or `nmcli`.

For backlight, load, and meminfo, it reads directly from `/sys/class/backlight`, `/proc/loadavg` and `/proc/meminfo`.

On my x86-64 computer, it compiles to a single binary that is only `16K` large using `clang-11`.

## Runtime Dependency
 - libupower-glib.so.3
 - libasound.so.2
 - libnm.so.0
 - libjson-c.so.5 (the same json library as sway and swaybar)

## Build

Install `clang`, `lld`, `make` and `pkg-config`, then run

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

Config file format:

    {
        "name": {
            "format": "##RRGGBBA",
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
For volume, you can also set property mix_name and card.
NOTE that property "format" is now only supported by time.

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
```

### Use `swaybar` in `sway`

```
bar {
    status_command swaystatus
}
```

[screenshot]: https://raw.githubusercontent.com/NobodyXu/swaystatus/main/screenshot.png
[`strftime`]: https://man7.org/linux/man-pages/man3/strftime.3.html
