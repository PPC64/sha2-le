.PHONY: test

all: sha256 sha512

sha256: sha256.c sha2.h
	gcc -g -std=c99 -o sha256 sha256.c

sha512: sha512.c sha2.h
	gcc -g -std=c99 -o sha512 sha512.c

tests: tests.c sha2.h
	gcc -g -std=c99 -o tests tests.c

test: tests
	./tests
