# Path
BUILD_DIR ?= .
TARGET_DIR := /usr/local

# Features
PYTHON ?= true
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

ifeq ($(PYTHON), true)

ifneq ($(shell python3 -c 'import sys; print(sys.version_info >= (3, 7))'), True)
	$(error Minimum python3 version is 3.7)
endif

	CFLAGS += $(shell python3-config --includes) -DUSE_PYTHON
	LIBS += $(shell python3-config --ldflags --embed)
endif

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

compile_flags.txt: Makefile
	echo '-xc++' $(CXXFLAGS) $(CFLAGS) | sed 's/-I/-I /g' | xargs -n1 echo | tee $@

compile_commands.json: Makefile
	rm -rf /tmp/swaystatus_build_dir/
	mkdir -p /tmp/swaystatus_build_dir/
	BUILD_DIR=/tmp/swaystatus_build_dir/ PYTHON=true DEBUG=true compiledb make -j $(shell nproc)
	rm -rf /tmp/swaystatus_build_dir/

### Convenient target for automatically running debug/release builds
### Move from run_all_builds.sh to further parallel the builds
all_builds: debug_build release_build release_no_py_build

debug_build:
	mkdir -p $@
	$(MAKE) BUILD_DIR=$@ DEBUG=true  PYTHON=true

release_build:
	mkdir -p $@
	$(MAKE) BUILD_DIR=$@ DEBUG=false PYTHON=true

release_no_py_build:
	mkdir -p $@
	$(MAKE) BUILD_DIR=$@ DEBUG=false PYTHON=false

## Dependencies
$(BUILD_DIR)/swaystatus.o: help.h utility.h printer.hpp process_configuration.h poller.h

$(BUILD_DIR)/process_configuration.o: utility.h $(wildcard print_*.h) handle_click_events.h

$(filter $(BUILD_DIR)/print_%.o,$(OBJS)): printer.hpp process_configuration.h

$(BUILD_DIR)/print_volume.o: alsa.h

printer.hpp: dep/fmt/include/fmt/core.h fmt_config.h
	touch $@
$(BUILD_DIR)/printer.o: dep/fmt/include/fmt/format.h dep/fmt/include/fmt/format-inl.h fmt_config.h

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

$(BUILD_DIR)/handle_click_events.o: poller.h utility.h python3.hpp

$(BUILD_DIR)/print_brightness.o: Conditional.hpp
$(BUILD_DIR)/print_battery.o: Conditional.hpp
$(BUILD_DIR)/print_memory_usage.o: mem_size_t.hpp LazyEval.hpp
$(BUILD_DIR)/print_network_interfaces.o: Conditional.hpp networking.hpp

install: $(BUILD_DIR)/swaystatus
	cp -f $(BUILD_DIR)/swaystatus $(TARGET_DIR)/bin/

install_release_build: release_build
	$(MAKE) BUILD_DIR=release_build DEBUG=false PYTHON=true install

clean:
	rm -rf $(OBJS) $(BUILD_DIR)/swaystatus *_build/

.PHONY: all_builds debug_build release_build release_no_py_build install clean
