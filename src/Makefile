# Path
BUILD_DIR ?= .
TARGET_DIR := /usr/local

# Features
DEBUG ?= false

# compiler and flags
CC := clang
CXX := clang++

ifeq ($(DEBUG), false)
	CFLAGS := -march=native -mtune=native -Oz -Wall -flto
	CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables

	CXXFLAGS := -fno-exceptions -fno-rtti

	LDFLAGS := -s -flto -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3,--as-needed
else
	CFLAGS := -Og -g -Wall
endif

CFLAGS += $(shell pkg-config --cflags alsa json-c)
LIBS := $(shell pkg-config --libs alsa json-c) -ldl -lsensors

## Objects to build
C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.o)

CXX_SRCS := $(wildcard *.cc)
CXX_OBJS := $(CXX_SRCS:.cc=.o)

OBJS := $(addprefix $(BUILD_DIR)/, $(C_OBJS) $(CXX_OBJS))

## Build rules
.SECONDEXPANSION:

$(BUILD_DIR)/swaystatus: $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@

$(BUILD_DIR)/%.o: %.c $$(wildcard %.h) Makefile
	$(CC) -std=c11 -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.cc $$(wildcard %.h) $$(wildcard %.hpp) Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

## Dependencies
$(BUILD_DIR)/swaystatus.o: help.h utility.h printer.hpp process_configuration.h poller.h

$(BUILD_DIR)/process_configuration.o: utility.h $(wildcard print_*.h)

$(filter $(BUILD_DIR)/print_%.o,$(OBJS)): printer.hpp process_configuration.h

$(BUILD_DIR)/print_volume.o: alsa.h

printer.hpp: dep/fmt/include/fmt/core.h fmt_config.h
	touch $@
printer.cc: dep/fmt/include/fmt/format.h dep/fmt/include/fmt/format-inl.h fmt_config.h

fmt_utility.hpp: dep/fmt/include/fmt/format.h fmt_config.h
	touch $@

mem_size_t.hpp: dep/fmt/include/fmt/format.h
	touch $@
LazyEval.hpp: dep/fmt/include/fmt/format.h
	touch $@
Conditional.hpp: dep/fmt/include/fmt/format.h fmt_utility.hpp
	touch $@

networking.hpp: dep/fmt/include/fmt/format.h fmt_config.h
	touch $@
$(BUILD_DIR)/networking.o: utility.h fmt_utility.hpp Conditional.hpp mem_size_t.hpp dep/fmt/include/fmt/core.h

sensors.hpp: dep/fmt/include/fmt/format.h fmt_config.h

$(BUILD_DIR)/print_brightness.o: Conditional.hpp
$(BUILD_DIR)/print_battery.o: Conditional.hpp
$(BUILD_DIR)/print_memory_usage.o: mem_size_t.hpp LazyEval.hpp
$(BUILD_DIR)/print_network_interfaces.o: Conditional.hpp networking.hpp

install: $(BUILD_DIR)/swaystatus
	cp -f $(BUILD_DIR)/swaystatus $(TARGET_DIR)/bin/

clean:
	rm -f $(OBJS) $(BUILD_DIR)/swaystatus

.PHONY: install clean
