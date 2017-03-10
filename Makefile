.PHONY: all tests clean perf

# Only compile the assembly variants for ppc64le architecture
ARCH:=$(shell uname -m)

BIN_DIR=./bin
CC=gcc
COMPILERS=gcc-4.8 gcc-4.9 gcc-5 gcc-6
OPT=-O3

ifeq (x$(ARCH),xppc64le)
CFLAGS=$(OPT) -g -Wall -Werror -maltivec -mcpu=power8 -std=c99
else
CFLAGS=$(OPT) -g -Wall -Werror -std=c99
endif

PERF_TXT=$(BIN_DIR)/perfexample.txt
# Number of perf stat iterations
PERF_ITERS=10

OBJS_256  = $(BIN_DIR)/sha256_compress_c_$(CC).o  \
            $(BIN_DIR)/sha256_c_$(CC).o           \
            $(BIN_DIR)/sha256_$(CC).o             \
            $(BIN_DIR)/sha256_common_$(CC).o
OBJS_512  = $(BIN_DIR)/sha512_compress_c_$(CC).o  \
            $(BIN_DIR)/sha512_c_$(CC).o           \
            $(BIN_DIR)/sha512_$(CC).o             \
            $(BIN_DIR)/sha512_common_$(CC).o
BINS_256  = $(BIN_DIR)/sha256_libcrypto_$(CC)     \
            $(BIN_DIR)/sha256_c_$(CC)
BINS_512  = $(BIN_DIR)/sha512_libcrypto_$(CC)     \
            $(BIN_DIR)/sha512_c_$(CC)
TESTS_256 =
TESTS_512 =

ifeq (x$(ARCH),xppc64le)
OBJS_256  += $(BIN_DIR)/sha256_compress_$(CC).o   \
						 $(BIN_DIR)/test256_$(CC).o
OBJS_512  += $(BIN_DIR)/sha512_compress_$(CC).o   \
						 $(BIN_DIR)/test512_$(CC).o
BINS_256  += $(BIN_DIR)/sha256_$(CC)
BINS_512  += $(BIN_DIR)/sha512_$(CC)
TESTS_256 += $(BIN_DIR)/test256_$(CC)
TESTS_512 += $(BIN_DIR)/test512_$(CC)
endif

OBJS = $(OBJS_256) $(OBJS_512)
BINS = $(BINS_256) $(BINS_512)
TESTS = $(TESTS_256) $(TESTS_512)

all:
	mkdir -p $(BIN_DIR)
	@for i in $(COMPILERS); do        \
		$(MAKE) all-compiler CC=$${i};  \
	done

all-compiler: $(BINS)

# Objects - Not using %.o as X64 wouldn't compile
$(BIN_DIR)/sha256_compress_c_$(CC).o: sha2_compress_c.c base-types.h sha2_compress.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_compress_c_$(CC).o: sha2_compress_c.c base-types.h sha2_compress.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_c_$(CC).o: sha2_compress_c.c sha2_common.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_c_$(CC).o: sha2_compress_c.c sha2_common.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_$(CC).o: sha2.c sha2_common.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_$(CC).o: sha2.c sha2_common.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_common_$(CC).o: sha2_common.c sha2_compress.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_common_$(CC).o: sha2_common.c sha2_compress.h base-types.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@

ifeq (x$(ARCH),xppc64le)
$(BIN_DIR)/sha256_compress_$(CC).o: sha256_compress.c base-types.h sha2_compress.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_compress_$(CC).o: sha512_compress.c base-types.h sha2_compress.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/test256_$(CC).o: tests.c base-types.h sha2_common.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/test512_$(CC).o: tests.c base-types.h sha2_common.h
	$(CC) $(CFLAGS) -c $< -DSHA_BITS=512 -o $@
endif


# Binaries
$(BIN_DIR)/sha256_libcrypto_$(CC): sha2.c
	$(CC) $(CFLAGS) $^ -DSHA_BITS=256 -DLIBCRYPTO -o $@ -lcrypto
$(BIN_DIR)/sha512_libcrypto_$(CC): sha2.c
	$(CC) $(CFLAGS) $^ -DSHA_BITS=512 -DLIBCRYPTO -o $@ -lcrypto

$(BIN_DIR)/sha256_c_$(CC): $(BIN_DIR)/sha256_c_$(CC).o $(BIN_DIR)/sha256_$(CC).o $(BIN_DIR)/sha256_common_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@
$(BIN_DIR)/sha512_c_$(CC): $(BIN_DIR)/sha512_c_$(CC).o $(BIN_DIR)/sha512_$(CC).o $(BIN_DIR)/sha512_common_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@

ifeq (x$(ARCH),xppc64le)
$(BIN_DIR)/sha256_$(CC): $(BIN_DIR)/sha256_$(CC).o $(BIN_DIR)/sha256_compress_$(CC).o $(BIN_DIR)/sha256_common_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@
$(BIN_DIR)/sha512_$(CC): $(BIN_DIR)/sha512_$(CC).o $(BIN_DIR)/sha512_compress_$(CC).o $(BIN_DIR)/sha512_common_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/test256_$(CC): $(BIN_DIR)/test256_$(CC).o $(BIN_DIR)/sha256_common_$(CC).o $(BIN_DIR)/sha256_compress_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@
$(BIN_DIR)/test512_$(CC): $(BIN_DIR)/test512_$(CC).o $(BIN_DIR)/sha512_common_$(CC).o $(BIN_DIR)/sha512_compress_$(CC).o
	$(CC) $(CFLAGS) $^ -o $@
endif

test-compiler: $(BINS) $(TESTS)
	@echo "======================================================================="
	@echo "Testing $(CC)"
	@echo "======================================================================="
ifeq (x$(ARCH),xppc64le)
	$(BIN_DIR)/test256_$(CC)
	$(BIN_DIR)/test512_$(CC)
endif
	CC=$(CC) ./blackbox-test.sh

test: all
	@for i in $(COMPILERS); do         \
		$(MAKE) test-compiler CC=$${i};  \
	done

perf-run: all
	@# Generating a 56M file
	@rm -f $(PERF_TXT)
	@for i in `seq 99`; do        \
		seq 1 100000 >> $(PERF_TXT); \
	done
	@echo "======================================================================="
	@echo "$(CC) perf run:"
	@echo "======================================================================="
	@echo "SHA 256"
ifeq (x$(ARCH),xppc64le)
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_$(CC)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
endif
	@echo -n "Libcrypto implementation:  "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_libcrypto_$(CC)     $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_c_$(CC)             $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

	@echo "\nSHA 512"
ifeq (x$(ARCH),xppc64le)
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_$(CC)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
endif
	@echo -n "Libcrypto implementation:  "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_libcrypto_$(CC)     $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_c_$(CC)             $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

perf:
	@for i in $(COMPILERS); do       \
		$(MAKE) -s perf-run CC=$${i};  \
	done

clean-compiler:
	rm -f $(BINS) $(TEST)
	rm -f $(PERF_TXT)

clean:
	@for i in $(COMPILERS); do             \
		$(MAKE) -s clean-compiler CC=$${i};  \
	done
	rm -rf $(BIN_DIR)
