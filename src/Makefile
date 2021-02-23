CC := clang
CXX := clang++

CFLAGS := -march=native -mtune=native -Oz -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables

CFLAGS += $(shell pkg-config --cflags alsa libnm json-c)

CXXFLAGS := -fno-exceptions -fno-rtti

LDFLAGS := -s -flto -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3

LIBS := $(shell pkg-config --libs alsa libnm json-c) -ldl

TARGET_DIR := /usr/local

## Objects to build
C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.o)

CXX_SRCS := $(wildcard *.cc)
CXX_OBJS := $(CXX_SRCS:.cc=.o)

OBJS := $(C_OBJS) $(CXX_OBJS)

## Build rules
.SECONDEXPANSION:

swaystatus: $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.c $$(wildcard %.h) Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

%.o: %.cc $$(wildcard %.h) $$(wildcard %.hpp) Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

## Dependencies
swaystatus.o: help.h utility.h printer.hpp process_configuration.h $(wildcard *.h)

$(filter print_%.o,$(OBJS)): printer.hpp

print_volume.o: alsa.h

printer.hpp: dep/fmt/include/fmt/core.h fmt_config.h
printer.cc: dep/fmt/include/fmt/format.h dep/fmt/include/fmt/format-inl.h fmt_config.h

mem_size_t.hpp: dep/fmt/include/fmt/format.h
LazyEval.hpp: dep/fmt/include/fmt/format.h
Conditional.hpp: dep/fmt/include/fmt/format.h

print_brightness.o: Conditional.hpp
print_battery.o: Conditional.hpp
print_memory_usage.o: mem_size_t.hpp LazyEval.hpp
print_network_interfaces.o: NMIPConfig.hpp

install: swaystatus
	cp -f swaystatus $(TARGET_DIR)/bin/

clean:
	rm -f $(OBJS) swaystatus

.PHONY: install clean
