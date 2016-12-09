.PHONY: test

all: sha2 tests

sha2: sha2.c sha2.h
	gcc -g -std=c99 -o sha2 sha2.c

tests: tests.c sha2.h
	gcc -g -std=c99 -o tests tests.c

test: tests
	./tests
