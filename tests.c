#include "sha2.h"

void print_error() {
	printf("ERROR!\n");
	exit(1);
}

int main () {
	if(rotate_right(1, 2) != 0x40000000) print_error();
	if(rotate_right(2, 2) != 0x80000000) print_error();
	if(calc_s0(0x80000) != 0x11002) print_error();
	if(calc_s1(0x800000) != 0x2050) print_error();
	if(calc_S0(0x8000000) != 0x2004020) print_error();
	if(calc_S1(0x8000000) != 0x210004) print_error();
	if(calc_ch(0xABABABAB,0xCDCDCDCD,0xEFEFEFEF) != 0xCDCDCDCD) print_error();
	if(calc_maj(0xABABABAB,0xCDCDCDCD,0xEFEFEFEF) != 0xEFEFEFEF) print_error();

	return 0;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
