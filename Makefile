.PHONY: all test tests clean

all: sha256 sha256_hw sha512 sha512_hw

sha256:     sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=256 -DUSE_HW_VECTOR=0 -o sha256

sha256_hw:  sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=256 -DUSE_HW_VECTOR=1 -o sha256_hw

sha512:     sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=512 -DUSE_HW_VECTOR=0 -o sha512

sha512_hw:  sha2.c sha2.h
	gcc -g -std=c99 sha2.c  -DSHA_BITS=512 -DUSE_HW_VECTOR=1 -o sha512_hw

test256:    tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=256 -DUSE_HW_VECTOR=0 -o tests256

test256_hw: tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=256 -DUSE_HW_VECTOR=1 -o tests256_hw

test512:    tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=512 -DUSE_HW_VECTOR=0 -o tests512

test512_hw: tests.c sha2.h
	gcc -g -std=c99 tests.c -DSHA_BITS=512 -DUSE_HW_VECTOR=1 -o tests512_hw

test: sha256 sha256_hw sha512 sha512_hw test256 test256_hw test512 test512_hw
	./tests256
	./tests256_hw
	./tests512
	./tests512_hw
	./blackbox-test.sh

tests: test

clean:
	rm -f sha256 sha256_hw sha512 sha512_hw tests256 tests256_hw tests512 tests512_hw
