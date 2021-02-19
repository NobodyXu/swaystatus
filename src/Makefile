CC := clang
CXX := clang++

CFLAGS := -march=native -mtune=native -Oz -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables

CFLAGS += $(shell pkg-config --cflags upower-glib alsa libnm json-c)

CXXFLAGS := -fno-exceptions -fno-rtti

LDFLAGS := -s -flto -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3

LIBS := $(shell pkg-config --libs upower-glib alsa libnm json-c)

TARGET_DIR := /usr/local

## Objects to build
C_SRCS := $(wildcard *.c)
CXX_SRCS := $(wildcard *.cc)
OBJS := $(C_SRCS:.c=.o) $(CXX_SRCS:.cc=.o)

## Build rules
swaystatus: $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.c Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

%.o: %.cc Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

%.o: %.c %.h Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

%.o: %.cc %.h Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

%.o: %.cc %.hpp Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

## Dependencies
swaystatus.o: help.h

print_%.o: print_%.cc printer.hpp
print_%.o: print_%.c printer.hpp

install: swaystatus
	cp -f swaystatus $(TARGET_DIR)/bin/

clean:
	rm -f $(OBJS) swaystatus

.PHONY: install clean
