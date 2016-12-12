.PHONY: test tests clean

all: sha256 sha512

sha256: sha256.c sha2.h
	gcc -g -std=c99 -o sha256 sha256.c
	gcc -g -std=c99 -DIN_ASM -o sha256_asm sha256.c

sha512: sha512.c sha2.h
	gcc -g -std=c99 -o sha512 sha512.c
	gcc -g -std=c99 -DIN_ASM -o sha512_asm sha512.c

test256: tests.c sha2.h
	gcc -DSHA_BITS=256 -g -std=c99 -o tests256 tests.c
	gcc -DSHA_BITS=256 -DIN_ASM -g -std=c99 -o tests256_asm tests.c

test512: tests.c sha2.h
	gcc -DSHA_BITS=512 -g -std=c99 -o tests512 tests.c
	gcc -DSHA_BITS=512 -DIN_ASM -g -std=c99 -o tests512_asm tests.c

tests: test256 test512

test: tests
	./tests256
	./tests256_asm
	./tests512
	./tests512_asm
	./blackbox-test.sh

clean:
	rm sha256 sha512 tests256 tests512
