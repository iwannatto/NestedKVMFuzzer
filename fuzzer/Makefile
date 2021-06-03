fuzznetlink: src/*h src/*c Makefile
	AFLplusplus/afl-clang-fast -g -O2 -Wall -Wextra \
		src/kcov.c \
		src/forksrv.c \
		src/siphash.c \
		src/fuzznetlink.c \
		-o $@

		# src/utils.c \
		# src/namespace.c \


.PHONY: format
format:
	clang-format -i src/*.c src/*.h
