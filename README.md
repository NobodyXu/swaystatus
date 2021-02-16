# swaystatus

A lightweight yet feature-rich status bar for i3 or sway.

![screenshot]

It is written completely in C to make it as lightweight as possible and specifically, to avoid creating new processes every second as in bash script.

It uses libraries like `libupower-glib`, `libasound` and `libnm` to retrieve battery, volume and network information as opposed to using `upower`, `amixer` or `nmcli`.

For backlight, load, and meminfo, it reads directly from `/sys/class/backlight`, `/proc/loadavg` and `/proc/meminfo`.

On my x86-64 computer, it compiles to a single binary that is only `16K` large using `clang-11`.

## Runtime Dependency
 - libupower-glib.so.3
 - libasound.so.2
 - libnm.so.0

## Build

Install `clang`, `lld`, `make` and `pkg-config`, then run

```
git clone https://github.com/NobodyXu/swaystatus
cd swaystatus/src
make -j $(nproc)
```

To install, run `sudo make install`, which by default will install a single binary `swaystatus` to `/usr/local/bin`.

## Usage

`swaystatus` accepts one optional arg for formating date and time, which will be passed to [`strftime`] directly.

[screenshot]: https://raw.githubusercontent.com/NobodyXu/swaystatus/main/screenshot.png
[`strftime`]: https://man7.org/linux/man-pages/man3/strftime.3.html
