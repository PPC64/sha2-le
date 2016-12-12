// Define these macros when compiling (See Makefile):
//
// SHA_BITS      256 or 512
// USE_HW_VECTOR 0 or 1

#include "sha2.h"

int main (int argc, char *argv[]) {
    return sha2(argc, argv);
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
