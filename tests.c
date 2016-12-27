#include "sha2.h"

static int error_occured = 0;

void print_error(int line) {
	printf("ERROR on line %d!\n", line);
	error_occured = 1;
}

void print_error_arg(int line, size_t arg) {
	printf("ERROR on line %d! Output: %zu\n", line, arg);
	error_occured = 1;
}

int main () {

#if (LOW_LEVEL != 1) && (LOW_LEVEL != 2)
    base_type n, b;
#endif
#if SHA_BITS==256
#if (LOW_LEVEL != 1) && (LOW_LEVEL != 2)
    n = 1; b= 2;
	if(ROTR(n, b) != 0x40000000) print_error(__LINE__);
    n = 2; b = 2;
    if(ROTR(n, b) != 0x80000000) print_error(__LINE__);
	if(SIGMA0(0x80000) != 0x11002) print_error(__LINE__);
	if(SIGMA1(0x800000) != 0x2050) print_error(__LINE__);
	if(BIGSIGMA0(0x8000000) != 0x2004020) print_error(__LINE__);
    if(BIGSIGMA1(0x8000000) != 0x210004) print_error(__LINE__);
#endif

	size_t size, padded_size;
	size =  3;
	padded_size = calculate_padded_msg_size(size);            if (padded_size !=  64) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size !=  64) print_error_arg(__LINE__, padded_size);
	size = 63;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	size = 64;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	size = 65;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	// testing around 5G
	size = 5368709119;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	size = 5368709120;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	size = 5368709121;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
#endif

#if SHA_BITS==512
#if (LOW_LEVEL != 1) && (LOW_LEVEL != 2)
    n = 1; b = 2;
	if(ROTR(n, b) != 0x4000000000000000) print_error_arg(__LINE__, ROTR(1,2));
    n = 2; b = 2;
	if(ROTR(n, b) != 0x8000000000000000) print_error_arg(__LINE__, ROTR(2,2));
	if(SIGMA0(0x80000) != 0x41800) print_error(__LINE__);
    if(SIGMA1(0x4000000000000000) != 0x100080000000002) print_error(__LINE__);
	if(BIGSIGMA0(0x4000000000000000) != 0x410800000) print_error(__LINE__);
	if(BIGSIGMA1(0x4000000000000000) != 0x1100000200000) print_error(__LINE__);
#endif

	size_t size, padded_size;
	size =  3;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 128) print_error_arg(__LINE__, padded_size);
	size = 127;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	size = 128;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	size = 129;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 256) print_error_arg(__LINE__, padded_size);
	// testing around 5G
	size = 5368709119;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
	size = 5368709120;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
	size = 5368709121;
	padded_size = calculate_padded_msg_size(size);            if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
	padded_size = calculate_padded_msg_size_FIPS_180_4(size); if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
#endif
	return error_occured;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
