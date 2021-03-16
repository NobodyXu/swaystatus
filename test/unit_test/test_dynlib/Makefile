CC := clang

all: libtest_lib.so
	mkdir -p test_dir/
	cp $< test_dir/libtest_lib2.so

libtest_lib.so: lib.c Makefile
	$(CC) -fPIC -shared $< -o $@
