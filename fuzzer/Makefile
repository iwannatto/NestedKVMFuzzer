fuzznetlink: src/*h src/*c Makefile
	AFLplusplus/afl-clang-fast -g -O2 -Wall -Wextra -DAFL \
		src/fuzznetlink.c \
		-o $@

standalone_fuzznetlink: src/*h src/*c Makefile
	AFLplusplus/afl-clang-fast -g -O2 -Wall -Wextra \
		src/fuzznetlink.c \
		-o $@

.PHONY: format
format:
	clang-format -i src/*.c src/*.h
