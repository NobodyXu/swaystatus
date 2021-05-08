CXX := clang++

CFLAGS := -Og -g -Wall

LIBS := $(shell pkg-config --libs alsa json-c) -ldl -lsensors
LIBS += $(shell python3-config --ldflags --embed)

## Objects to build
C_SRCS := $(shell find -name 'test_*.c')
C_OBJS := $(C_SRCS:.c=.out)

CXX_SRCS := $(shell find -name 'test_*.cc')
CXX_OBJS := $(CXX_SRCS:.cc=.out)

OBJS := $(C_OBJS) $(CXX_OBJS)

DEPS := $(filter-out build_dir/swaystatus.o, $(shell find build_dir/ -name '*.o'))

## Build rules
all:
	mkdir -p build_dir
	mkdir -p build_dir/Callback
	mkdir -p build_dir/formatting
	mkdir -p build_dir/modules
	BUILD_DIR=${PWD}/build_dir PYTHON=true DEBUG=true $(MAKE) -C ../../src/
	$(MAKE) build
	./run_tests.sh $(OBJS)

build: $(OBJS)

%.out: %.cc $(DEPS) Makefile
	$(CXX) -std=c++17 $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $< $(DEPS)
	# Allow subdir to have their own Makefile for building like library.
	([ -f "$$(dirname $<)/Makefile" ] && $(MAKE) -C $$(dirname $<)) || echo

compile_commands.json: Makefile
	compiledb make build -j $(shell nproc)

clean:
	rm -rf $(OBJS) build_dir/

.PHONY: all build clean
