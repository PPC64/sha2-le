.PHONY: test

all: sha256

sha256: sha256.c sha2.h
	gcc -g -std=c99 -o sha256 sha256.c

tests: tests.c sha2.h
	gcc -g -std=c99 -o tests tests.c

test: tests
	./tests
