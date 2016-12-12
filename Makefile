.PHONY: all test tests clean

BIN_DIR=./bin

all: sha256 sha256_hw sha512 sha512_hw

sha256:     sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=256 -DUSE_HW_VECTOR=0 -o $(BIN_DIR)/sha256

sha256_hw:  sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=256 -DUSE_HW_VECTOR=1 -o $(BIN_DIR)/sha256_hw

sha512:     sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=512 -DUSE_HW_VECTOR=0 -o $(BIN_DIR)/sha512

sha512_hw:  sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=512 -DUSE_HW_VECTOR=1 -o $(BIN_DIR)/sha512_hw

test256:    tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=256 -DUSE_HW_VECTOR=0 -o $(BIN_DIR)/tests256

test256_hw: tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=256 -DUSE_HW_VECTOR=1 -o $(BIN_DIR)/tests256_hw

test512:    tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=512 -DUSE_HW_VECTOR=0 -o $(BIN_DIR)/tests512

test512_hw: tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=512 -DUSE_HW_VECTOR=1 -o $(BIN_DIR)/tests512_hw

test: sha256 sha256_hw sha512 sha512_hw test256 test256_hw test512 test512_hw
	./$(BIN_DIR)/tests256
	./$(BIN_DIR)/tests256_hw
	./$(BIN_DIR)/tests512
	./$(BIN_DIR)/tests512_hw
	./blackbox-test.sh

tests: test

clean:
	rm -f $(BIN_DIR)/sha256 $(BIN_DIR)/sha256_hw $(BIN_DIR)/sha512 $(BIN_DIR)/sha512_hw $(BIN_DIR)/tests256 $(BIN_DIR)/tests256_hw $(BIN_DIR)/tests512 $(BIN_DIR)/tests512_hw
