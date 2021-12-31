qemu_wrapper: src/*h src/*c Makefile
	AFLplusplus/afl-clang-fast -g -O2 -Wall -Wextra -DAFL \
		src/qemu_wrapper.c \
		-o $@

standalone_qemu_wrapper: src/*h src/*c Makefile
	AFLplusplus/afl-clang-fast -g -O2 -Wall -Wextra \
		src/qemu_wrapper.c \
		-o $@

.PHONY: format
format:
	clang-format -i src/*.c src/*.h
