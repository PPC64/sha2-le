.PHONY: test

all: sha256 sha512

sha256: sha256.c sha2.h
	gcc -g -std=c99 -o sha256 sha256.c

sha512: sha512.c sha2.h
	gcc -g -std=c99 -o sha512 sha512.c

tests: tests.c sha2.h
	gcc -DSHA_BITS=256 -g -std=c99 -o tests256 tests.c
	gcc -DSHA_BITS=512 -g -std=c99 -o tests512 tests.c

test: tests
	./tests256
	./tests512
