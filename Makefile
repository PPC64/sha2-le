.PHONY: all test tests clean

CFLAGS=-g -Wall
BIN_DIR=./bin

all: bin/sha256 bin/sha256_ll_intrinsics bin/sha256_ll_asm bin/sha512 bin/sha512_ll_intrinsics bin/sha512_ll_asm

bin/sha256:     sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=256 -DLOW_LEVEL=0 -o $(BIN_DIR)/sha256

bin/sha256_ll_intrinsics:  sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=256 -DLOW_LEVEL=1 -o $(BIN_DIR)/sha256_ll_intrinsics

bin/sha256_ll_asm:  sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=256 -DLOW_LEVEL=2 -o $(BIN_DIR)/sha256_ll_asm

bin/sha512:     sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=512 -DLOW_LEVEL=0 -o $(BIN_DIR)/sha512

bin/sha512_ll_intrinsics:  sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=512 -DLOW_LEVEL=1 -o $(BIN_DIR)/sha512_ll_intrinsics

bin/sha512_ll_asm:  sha2.c sha2.h
	gcc $(CFLAGS) -std=c99 sha2.c  -DSHA_BITS=512 -DLOW_LEVEL=2 -o $(BIN_DIR)/sha512_ll_asm

bin/test256:    tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=256 -DLOW_LEVEL=0 -o $(BIN_DIR)/tests256

bin/test256_ll_intrinsics: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=256 -DLOW_LEVEL=1 -o $(BIN_DIR)/tests256_ll_intrinsics

bin/test256_ll_asm: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=256 -DLOW_LEVEL=2 -o $(BIN_DIR)/tests256_ll_asm

bin/test512:    tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=512 -DLOW_LEVEL=0 -o $(BIN_DIR)/tests512

bin/test512_ll_intrinsics: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=512 -DLOW_LEVEL=1 -o $(BIN_DIR)/tests512_ll_intrinsics

bin/test512_ll_asm: tests.c sha2.h
	gcc $(CFLAGS) -std=c99 tests.c -DSHA_BITS=512 -DLOW_LEVEL=2 -o $(BIN_DIR)/tests512_ll_asm

test: bin/sha256 bin/sha256_ll_intrinsics bin/sha256_ll_asm bin/sha512 bin/sha512_ll_intrinsics bin/sha512_ll_asm bin/test256 bin/test256_ll_intrinsics bin/test256_ll_asm bin/test512 bin/test512_ll_intrinsics bin/test512_ll_asm
	./$(BIN_DIR)/tests256
	./$(BIN_DIR)/tests256_ll_intrinsics
	./$(BIN_DIR)/tests256_ll_asm
	./$(BIN_DIR)/tests512
	./$(BIN_DIR)/tests512_ll_intrinsics
	./$(BIN_DIR)/tests512_ll_asm
	./blackbox-test.sh

tests: test

clean:
	rm -f $(BIN_DIR)/sha256 $(BIN_DIR)/sha256_ll_intrinsics $(BIN_DIR)/sha256_ll_asm $(BIN_DIR)/sha512 $(BIN_DIR)/sha512_ll_intrinsics $(BIN_DIR)/sha512_ll_asm $(BIN_DIR)/tests256 $(BIN_DIR)/tests256_ll_intrinsics $(BIN_DIR)/tests256_ll_asm $(BIN_DIR)/tests512 $(BIN_DIR)/tests512_ll_intrinsics $(BIN_DIR)/tests512_ll_asm
