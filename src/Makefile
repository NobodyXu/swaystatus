CC=clang

CFLAGS := -march=native -mtune=native -Oz -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables

CFLAGS += $(shell pkg-config --cflags upower-glib alsa libnm json-c)

LDFLAGS := -s -flto -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3

LIBS := $(shell pkg-config --libs upower-glib alsa libnm json-c)

TARGET_DIR := /usr/local

## Objects to build
SRCS := $(shell find . -name '*.c' -a ! -wholename './test/*' -a ! -wholename './benchmark/*')
OBJS := $(SRCS:.c=.o)

## Build rules
swaystatus: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.c Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

%.o: %.c %.h Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

## Dependencies
swaystatus.o: help.h

install: swaystatus
	cp -f swaystatus $(TARGET_DIR)/bin/

clean:
	rm -f $(OBJS) swaystatus

.PHONY: install clean
