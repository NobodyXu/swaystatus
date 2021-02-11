CC=clang

CFLAGS=-Oz -march=native -flto -fno-asynchronous-unwind-tables -fno-unwind-tables -fmerge-all-constants -fomit-frame-pointer
LDFLAGS=-s -fuse-ld=lld -Wl,-icf=all,--gc-sections,--plugin-opt=O3,-O3

print_time: print_time.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY: install
install: print_time
	cp -f print_time /usr/local/bin/
