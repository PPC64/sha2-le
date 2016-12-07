sha2: sha2.c sha2.h
	gcc -g -std=c99 -o sha2 sha2.c

tests: tests.c
	gcc -g -std=c99 -o tests tests.c
