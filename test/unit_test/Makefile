CXX := clang++

CFLAGS := -Og -g -Wall

LIBS := $(shell pkg-config --libs alsa json-c) -ldl -lsensors
LIBS += $(shell python3-config --ldflags --embed)

## Objects to build
C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.out)

CXX_SRCS := $(wildcard *.cc)
CXX_OBJS := $(CXX_SRCS:.cc=.out)

OBJS := $(C_OBJS) $(CXX_OBJS)

DEPS := $(filter-out build_dir/swaystatus.o, $(wildcard build_dir/*.o))

## Build rules
all:
	mkdir -p build_dir
	BUILD_DIR=${PWD}/build_dir PYTHON=true DEBUG=true $(MAKE) -C ../../src/
	$(MAKE) build
	../run_tests.sh $(OBJS)

build: $(OBJS)

test_%.out: test_%.cc $(DEPS) Makefile
	$(CXX) -std=c++17 $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $< $(DEPS)

compile_commands.json: Makefile
	compiledb make build -j $(shell nproc)

clean:
	rm -rf $(OBJS) build_dir/

.PHONY: all build clean
