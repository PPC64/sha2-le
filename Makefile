.PHONY: all tests clean perf

# Only compile the assembly variants for ppc64le architecture
ARCH:=$(shell uname -m)

BIN_DIR=./bin
CC=gcc
COMPILERS=gcc-4.8 gcc-4.9 gcc-5 gcc-6
CFLAGS=-g -O3 
ALL_CFLAGS=$(CFLAGS)

PERF_TXT=$(BIN_DIR)/perfexample.txt
# Number of perf stat iterations
PERF_ITERS=10

OBJS_256  = $(BIN_DIR)/sha256_compress_c_$(SUFFIX).o  \
            $(BIN_DIR)/sha256_c_$(SUFFIX).o           \
            $(BIN_DIR)/sha256_$(SUFFIX).o             \
            $(BIN_DIR)/sha256_common_$(SUFFIX).o
OBJS_512  = $(BIN_DIR)/sha512_compress_c_$(SUFFIX).o  \
            $(BIN_DIR)/sha512_c_$(SUFFIX).o           \
            $(BIN_DIR)/sha512_$(SUFFIX).o             \
            $(BIN_DIR)/sha512_common_$(SUFFIX).o
BINS_256  = $(BIN_DIR)/sha256_libcrypto_$(SUFFIX)     \
            $(BIN_DIR)/sha256_c_$(SUFFIX)
BINS_512  = $(BIN_DIR)/sha512_libcrypto_$(SUFFIX)     \
            $(BIN_DIR)/sha512_c_$(SUFFIX)
TESTS_256 =
TESTS_512 =

ifeq (x$(ARCH),xppc64le)
OBJS_256  += $(BIN_DIR)/sha256_compress_$(SUFFIX).o   \
             $(BIN_DIR)/test256_$(SUFFIX).o
OBJS_512  += $(BIN_DIR)/sha512_compress_$(SUFFIX).o   \
             $(BIN_DIR)/test512_$(SUFFIX).o
BINS_256  += $(BIN_DIR)/sha256_$(SUFFIX)
BINS_512  += $(BIN_DIR)/sha512_$(SUFFIX)
TESTS_256 += $(BIN_DIR)/test256_$(SUFFIX)
TESTS_512 += $(BIN_DIR)/test512_$(SUFFIX)
endif

OBJS = $(OBJS_256) $(OBJS_512)
BINS = $(BINS_256) $(BINS_512)
TESTS = $(TESTS_256) $(TESTS_512)

all:
	mkdir -p $(BIN_DIR)
	@for i in $(COMPILERS); do                                  \
		$(MAKE) all-compiler CC=$${i} SUFFIX=$$(basename $${i});  \
	done

all-compiler: $(BINS)

# Objects - Not using %.o as X64 wouldn't compile
$(BIN_DIR)/sha256_compress_c_$(SUFFIX).o: sha2_compress_c.c base-types.h sha2_compress.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_compress_c_$(SUFFIX).o: sha2_compress_c.c base-types.h sha2_compress.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_c_$(SUFFIX).o: sha2_compress_c.c sha2_common.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_c_$(SUFFIX).o: sha2_compress_c.c sha2_common.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_$(SUFFIX).o: sha2.c sha2_common.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_$(SUFFIX).o: sha2.c sha2_common.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/sha256_common_$(SUFFIX).o: sha2_common.c sha2_compress.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/sha512_common_$(SUFFIX).o: sha2_common.c sha2_compress.h base-types.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@

ifeq (x$(ARCH),xppc64le)
sha256_compress_ppc.s: common.m4 sha256_compress_ppc.m4 # Order matters!
	m4 $^ > $@
$(BIN_DIR)/sha256_compress_$(SUFFIX).o: sha256_compress_ppc.s
	$(CC) $(ALL_CFLAGS) -c $< -o $@
$(BIN_DIR)/sha512_compress_$(SUFFIX).o: sha512_compress.c base-types.h sha2_compress.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@

$(BIN_DIR)/test256_$(SUFFIX).o: tests.c base-types.h sha2_common.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=256 -o $@
$(BIN_DIR)/test512_$(SUFFIX).o: tests.c base-types.h sha2_common.h
	$(CC) $(ALL_CFLAGS) -c $< -DSHA_BITS=512 -o $@
endif


# Binaries
$(BIN_DIR)/sha256_libcrypto_$(SUFFIX): sha2.c
	$(CC) $(ALL_CFLAGS) $^ -DSHA_BITS=256 -DLIBCRYPTO -o $@ -lcrypto
$(BIN_DIR)/sha512_libcrypto_$(SUFFIX): sha2.c
	$(CC) $(ALL_CFLAGS) $^ -DSHA_BITS=512 -DLIBCRYPTO -o $@ -lcrypto

$(BIN_DIR)/sha256_c_$(SUFFIX): $(BIN_DIR)/sha256_c_$(SUFFIX).o $(BIN_DIR)/sha256_$(SUFFIX).o $(BIN_DIR)/sha256_common_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@
$(BIN_DIR)/sha512_c_$(SUFFIX): $(BIN_DIR)/sha512_c_$(SUFFIX).o $(BIN_DIR)/sha512_$(SUFFIX).o $(BIN_DIR)/sha512_common_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@

ifeq (x$(ARCH),xppc64le)
$(BIN_DIR)/sha256_$(SUFFIX): $(BIN_DIR)/sha256_$(SUFFIX).o $(BIN_DIR)/sha256_compress_$(SUFFIX).o $(BIN_DIR)/sha256_common_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@
$(BIN_DIR)/sha512_$(SUFFIX): $(BIN_DIR)/sha512_$(SUFFIX).o $(BIN_DIR)/sha512_compress_$(SUFFIX).o $(BIN_DIR)/sha512_common_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@

$(BIN_DIR)/test256_$(SUFFIX): $(BIN_DIR)/test256_$(SUFFIX).o $(BIN_DIR)/sha256_common_$(SUFFIX).o $(BIN_DIR)/sha256_compress_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@
$(BIN_DIR)/test512_$(SUFFIX): $(BIN_DIR)/test512_$(SUFFIX).o $(BIN_DIR)/sha512_common_$(SUFFIX).o $(BIN_DIR)/sha512_compress_$(SUFFIX).o
	$(CC) $(ALL_CFLAGS) $^ -o $@
endif

test-compiler: $(BINS) $(TESTS)
	@echo "======================================================================="
	@echo "Testing $(CC)"
	@echo "======================================================================="
ifeq (x$(ARCH),xppc64le)
	$(BIN_DIR)/test256_$(SUFFIX)
	$(BIN_DIR)/test512_$(SUFFIX)
endif
	CC=$(SUFFIX) ./blackbox-test.sh

test: all
	@for i in $(COMPILERS); do                                    \
		$(MAKE) test-compiler CC=$${i} SUFFIX=$$(basename $${i});  \
	done

perf-run: all
	@# Generating a 56M file
	@rm -f $(PERF_TXT)
	@for i in `seq 99`; do         \
		seq 1 100000 >> $(PERF_TXT); \
	done
	@echo "======================================================================="
	@echo "$(CC) perf run:"
	@echo "======================================================================="
	@echo "SHA 256"
ifeq (x$(ARCH),xppc64le)
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_$(SUFFIX)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
endif
	@echo -n "Libcrypto implementation:  "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_libcrypto_$(SUFFIX)     $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_c_$(SUFFIX)             $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

	@echo "\nSHA 512"
ifeq (x$(ARCH),xppc64le)
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_$(SUFFIX)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
endif
	@echo -n "Libcrypto implementation:  "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_libcrypto_$(SUFFIX)     $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_c_$(SUFFIX)             $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

perf:
	@for i in $(COMPILERS); do                                 \
		$(MAKE) -s perf-run CC=$${i} SUFFIX=$$(basename $${i});  \
	done

clean-compiler:
	rm -f $(BINS) $(TEST)
	rm -f $(PERF_TXT)

clean:
	@for i in $(COMPILERS); do    \
		$(MAKE) -s clean-compiler;  \
	done
	rm -rf $(BIN_DIR)
ifeq (x$(ARCH),xppc64le)
	rm -f sha256_compress_ppc.s
endif
