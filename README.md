# swaystatus

A lightweight yet feature-rich status bar for i3 or sway.

![screenshot]

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
