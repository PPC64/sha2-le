.PHONY: all test tests clean

CFLAGS=-g -Wall -fPIC -fno-omit-frame-pointer
BIN_DIR=./bin

BINS  = $(BIN_DIR)/sha256 $(BIN_DIR)/sha256_ll_intrinsics \
				$(BIN_DIR)/sha256_ll_asm $(BIN_DIR)/sha512 \
				$(BIN_DIR)/sha512_ll_intrinsics $(BIN_DIR)/sha512_ll_asm

TESTS = $(BIN_DIR)/test256 $(BIN_DIR)/test256_ll_intrinsics \
				$(BIN_DIR)/test256_ll_asm $(BIN_DIR)/test512 \
				$(BIN_DIR)/test512_ll_intrinsics $(BIN_DIR)/test512_ll_asm

all: $(BINS)

$(BIN_DIR)/sha256: sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/sha256_ll_intrinsics: sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/sha256_ll_asm: sha2.c sha2.h
	gcc $(CFLAGS)  -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/sha512: sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/sha512_ll_intrinsics: sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/sha512_ll_asm: sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/test256: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/test256_ll_intrinsics: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/test256_ll_asm: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=256 -DLOW_LEVEL=2 -o $@

$(BIN_DIR)/test512: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=0 -o $@

$(BIN_DIR)/test512_ll_intrinsics: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=1 -o $@

$(BIN_DIR)/test512_ll_asm: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 $< -DSHA_BITS=512 -DLOW_LEVEL=2 -o $@

test: $(BINS) $(TESTS)
	$(BIN_DIR)/test256
	$(BIN_DIR)/test256_ll_intrinsics
	$(BIN_DIR)/test256_ll_asm
	$(BIN_DIR)/test512
	$(BIN_DIR)/test512_ll_intrinsics
	$(BIN_DIR)/test512_ll_asm
	./blackbox-test.sh

tests: test

clean:
	rm -f $(BINS) $(TESTS)
