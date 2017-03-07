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

ifeq (x$(ARCH),xppc64le)
BINS  = $(BIN_DIR)/sha256_$(CC) $(BIN_DIR)/sha256_libcrypto_$(CC)      \
				$(BIN_DIR)/sha512_$(CC) $(BIN_DIR)/sha512_libcrypto_$(CC)
TESTS = $(BIN_DIR)/test256_$(CC) $(BIN_DIR)/test512_$(CC)
else
BINS  = $(BIN_DIR)/sha256_libcrypto_$(CC) $(BIN_DIR)/sha512_libcrypto_$(CC)
TESTS =
endif

all:
	mkdir -p $(BIN_DIR)
	@for i in $(COMPILERS); do	\
		$(MAKE) all-compiler CC=$${i};	\
	done

all-compiler: $(BINS)

ifeq (x$(ARCH),xppc64le)
$(BIN_DIR)/sha256_$(CC): sha2.c sha2.h sha2_common.c sha2_compress.h sha256_compress.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=256 -o $@
endif

$(BIN_DIR)/sha256_libcrypto_$(CC): sha2.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=256 -DLIBCRYPTO -o $@ -lcrypto

ifeq (x$(ARCH),xppc64le)
$(BIN_DIR)/sha512_$(CC): sha2.c sha2.h sha2_common.c sha2_compress.h sha512_compress.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=512 -o $@
endif

$(BIN_DIR)/sha512_libcrypto_$(CC): sha2.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=512 -DLIBCRYPTO -o $@ -lcrypto

$(BIN_DIR)/test256_$(CC): tests.c sha2.h sha2_common.c sha2_compress.h sha256_compress.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=256 -o $@

$(BIN_DIR)/test512_$(CC): tests.c sha2.h sha2_common.c sha2_compress.h sha512_compress.c
	$(CC) $(CFLAGS) $? -DSHA_BITS=512 -o $@

test-compiler: $(BINS) $(TESTS)
	@echo "======================================================================="
	@echo "Testing $(CC)"
	@echo "======================================================================="
ifeq (x$(ARCH),xppc64le)
	$(BIN_DIR)/test256_$(CC)
	$(BIN_DIR)/test512_$(CC)
endif
	CC=$(CC) ./blackbox-test.sh

test:
	@for i in $(COMPILERS); do	\
		$(MAKE) test-compiler CC=$${i};	\
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
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_libcrypto_$(CC) $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

	@echo "\nSHA 512"
ifeq (x$(ARCH),xppc64le)
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_$(CC)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
endif
	@echo -n "Libcrypto implementation:  "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_libcrypto_$(CC) $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

perf:
	@for i in $(COMPILERS); do	\
		$(MAKE) -s perf-run CC=$${i};	\
	done

clean-compiler:
	rm -f $(BINS) $(TEST)
	rm -f $(PERF_TXT)

clean:
	@for i in $(COMPILERS); do	\
		$(MAKE) -s clean-compiler CC=$${i};	\
	done
