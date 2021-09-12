# Path
BUILD_DIR ?= .
TARGET_DIR := /usr/local

# Features
PYTHON ?= true
DEBUG ?= false
EXCEPTION ?= true

# compiler and flags
CC := clang
CXX := clang++

CFLAGS := -Wall -Wextra -Werror

ifeq ($(DEBUG), false)
	CFLAGS += -O3 -flto
	CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables

	CXXFLAGS := -fno-rtti

	LDFLAGS := -march=native -s -flto -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3,--as-needed
else
	CFLAGS += -Og -g
endif

ifeq ($(EXCEPTION), false)
	CXXFLAGS += -fno-exceptions
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
C_SRCS := $(shell find -maxdepth 2 -name '*.c')
C_OBJS := $(C_SRCS:.c=.o)

CXX_SRCS := $(shell find -maxdepth 2 -name '*.cc')
CXX_OBJS := $(CXX_SRCS:.cc=.o)

OBJS := $(addprefix $(BUILD_DIR)/, $(C_OBJS) $(CXX_OBJS))

.DEFAULT_GOAL := $(BUILD_DIR)/swaystatus

## Automatic dependency building
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.Td
DEPFILES := $(OBJS:%.o=%.d)

## Dummy target for $(DEPFILES) when they are not present.
$(DEPFILES):
## Use wildcard so that nonexisitent dep files are ignored.
include $(wildcard $(DEPFILES))

## Build rules
.SECONDEXPANSION:

DEBUG:
	@echo "C_SRCS=$(C_SRCS), C_OBJS=$(C_OBJS)"
	@echo "CXX_SRCS=$(CXX_SRCS), CXX_OBJS=$(CXX_OBJS)"
	@echo "OBJS=$(OBJS)"

$(BUILD_DIR)/swaystatus: $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@

$(BUILD_DIR)/%.o: %.c $$(wildcard %.h) Makefile
	$(CC) -std=c11 -c $(CFLAGS) $(DEPFLAGS) -o $@ $<
	mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

$(BUILD_DIR)/%.o: %.cc $$(wildcard %.h) $$(wildcard %.hpp) Makefile
	$(CXX) -std=c++17 -c $(CXXFLAGS) $(CFLAGS) $(DEPFLAGS) -o $@ $<
	mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

compile_flags.txt: Makefile
	echo '-xc++' $(CXXFLAGS) $(CFLAGS) | sed 's/-I/-I /g' | xargs -n1 echo | tee $@

compile_commands.json: Makefile
	rm -rf /tmp/swaystatus_build_dir/
	mkdir -p /tmp/swaystatus_build_dir/
	BUILD_DIR=/tmp/swaystatus_build_dir/ PYTHON=true DEBUG=true compiledb make -j $(shell nproc)
	rm -rf /tmp/swaystatus_build_dir/

### Convenient target for automatically running debug/release builds
### Move from run_all_builds.sh to further parallel the builds
all_builds: release_build release_no_py_build

debug_build:
	mkdir -p $@
	mkdir -p $@/Callback
	mkdir -p $@/formatting
	mkdir -p $@/modules
	$(MAKE) BUILD_DIR=$@ DEBUG=true  PYTHON=true

release_build:
	mkdir -p $@
	mkdir -p $@/Callback
	mkdir -p $@/formatting
	mkdir -p $@/modules
	$(MAKE) BUILD_DIR=$@ DEBUG=false PYTHON=true

release_no_py_build:
	mkdir -p $@
	mkdir -p $@/Callback
	mkdir -p $@/formatting
	mkdir -p $@/modules
	$(MAKE) BUILD_DIR=$@ DEBUG=false PYTHON=false

install: $(BUILD_DIR)/swaystatus
	cp -f $(BUILD_DIR)/swaystatus $(TARGET_DIR)/bin/

install_release_build: release_build
	$(MAKE) BUILD_DIR=release_build DEBUG=false PYTHON=true install

clean:
	rm -rf $(OBJS) $(BUILD_DIR)/swaystatus $(DEPFILES)

.PHONY: DEBUG all_builds debug_build release_build release_no_py_build install install_release_build clean
