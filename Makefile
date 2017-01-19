.PHONY: all tests clean perf

BIN_DIR=./bin
CC=gcc
COMPILERS=gcc clang-3.8 clang-3.9
OPT=-O3
CFLAGS=$(OPT) -g -Wall -Werror -maltivec -std=c99
PERF_TXT=$(BIN_DIR)/perfexample.txt
# Number of perf stat iterations
PERF_ITERS=10

BINS  = $(BIN_DIR)/sha256_$(CC) $(BIN_DIR)/sha256_ll_intrinsics_$(CC) \
				$(BIN_DIR)/sha256_ll_asm_$(CC) $(BIN_DIR)/sha512_$(CC) \
				$(BIN_DIR)/sha512_ll_intrinsics_$(CC) $(BIN_DIR)/sha512_ll_asm_$(CC)

TESTS = $(BIN_DIR)/test256_$(CC) $(BIN_DIR)/test256_ll_intrinsics_$(CC) \
				$(BIN_DIR)/test256_ll_asm_$(CC) $(BIN_DIR)/test512_$(CC) \
				$(BIN_DIR)/test512_ll_intrinsics_$(CC) $(BIN_DIR)/test512_ll_asm_$(CC)

all:
	@for i in $(COMPILERS); do	\
		$(MAKE) all-compiler CC=$${i};	\
	done

all-compiler: $(BINS)

$(BIN_DIR)/sha256_$(CC): sha2.c sha2.h sha2_no_ll.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/sha256_ll_intrinsics_$(CC): sha2.c sha2.h sha2_ll_intrinsics.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/sha256_ll_asm_$(CC): sha2.c sha2.h sha2_ll_asm.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/sha512_$(CC): sha2.c sha2.h sha2_no_ll.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/sha512_ll_intrinsics_$(CC): sha2.c sha2.h sha2_ll_intrinsics.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/sha512_ll_asm_$(CC): sha2.c sha2.h sha2_ll_asm.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/test256_$(CC): tests.c sha2.h sha2_no_ll.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/test256_ll_intrinsics_$(CC): tests.c sha2.h sha2_ll_intrinsics.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/test256_ll_asm_$(CC): tests.c sha2.h sha2_ll_asm.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=256 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/test512_$(CC): tests.c sha2.h sha2_no_ll.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/test512_ll_intrinsics_$(CC): tests.c sha2.h sha2_ll_intrinsics.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/test512_ll_asm_$(CC): tests.c sha2.h sha2_ll_asm.h
	$(CC) $(CFLAGS) $< -DSHA_BITS=512 -DLOW_LEVEL=2 -o $@

test-compiler: $(BINS) $(TESTS)
	@echo "======================================================================="
	@echo "Testing $(CC)"
	@echo "======================================================================="
	$(BIN_DIR)/test256_$(CC)
	$(BIN_DIR)/test256_ll_intrinsics_$(CC)
	$(BIN_DIR)/test256_ll_asm_$(CC)
	$(BIN_DIR)/test512_$(CC)
	$(BIN_DIR)/test512_ll_intrinsics_$(CC)
	$(BIN_DIR)/test512_ll_asm_$(CC)
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
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_$(CC)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_ll_asm_$(CC)        $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "Intrinsics implementation: "
	@sudo perf stat -r $(PERF_ITERS) bin/sha256_ll_intrinsics_$(CC) $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

	@echo "\nSHA 512"
	@echo -n "C implementation:          "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_$(CC)               $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "ASM implementation:        "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_ll_asm_$(CC)        $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"
	@echo -n "Intrinsics implementation: "
	@sudo perf stat -r $(PERF_ITERS) bin/sha512_ll_intrinsics_$(CC) $(PERF_TXT) 2>&1 | grep "time elapsed" | sed -e "s/ time elapsed//"

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
