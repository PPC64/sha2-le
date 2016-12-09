#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t _h[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

uint32_t k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void calculate_padded_msg_size(size_t size, size_t *res) {
	// 64 bytes == 512 bits
	*res = 64 - (size % 64);
}

void pad(char *in, char *out, size_t size, size_t padded_size) {
	int i;
	for (i = 0; i < size; i ++)
		out[i] = in[i];

	// padding message with 1 and zeroes
	out[i++] = (char)(1 << 7);

	// reserve last 8 bytes for the size of the message
	for(; i < padded_size + size; i++)
		out[i] = 0;
}

void write_size(char *input, size_t size, size_t position) {
	uint32_t* total_size = (uint32_t*)&input[position];
	*total_size = (uint32_t)(size >> 32) * 8; // high word in bits
	*(++total_size) = (uint32_t)size * 8;     // low word in bits
}

uint32_t rotate_right(uint32_t num, uint32_t bits)
{
	return ((num >> bits) | (num << (32 -bits)));
}

uint32_t calc_s1(uint32_t e) {
	uint32_t tmp, tmp2, tmp3;
	tmp = rotate_right(e, 17);
	tmp2 = rotate_right(e, 19);
	tmp3 = e >> 10;
	return tmp ^ tmp2 ^ tmp3;
}
uint32_t calc_S1(uint32_t e) {
	uint32_t tmp, tmp2, tmp3;
	tmp = rotate_right(e, 6);
	tmp2 = rotate_right(e, 11);
	tmp3 = rotate_right(e, 25);
	return tmp ^ tmp2 ^ tmp3;
}
uint32_t calc_ch(uint32_t e, uint32_t f, uint32_t g) {
	uint32_t tmp, tmp2;
	tmp = e & f;
	tmp2 = ~e & g;
	return tmp ^ tmp2;
}
uint32_t calc_S0(uint32_t a) {
	uint32_t tmp, tmp2, tmp3;
	tmp = rotate_right(a, 2);
	tmp2 = rotate_right(a, 13);
	tmp3 = rotate_right(a, 22);
	return tmp ^ tmp2 ^ tmp3;
}
uint32_t calc_s0(uint32_t a) {
	uint32_t tmp, tmp2, tmp3;
	tmp = rotate_right(a, 7);
	tmp2 = rotate_right(a, 18);
	tmp3 = a >> 3;
	return tmp ^ tmp2 ^ tmp3;
}
uint32_t calc_maj(uint32_t a, uint32_t b, uint32_t c) {
	uint32_t tmp, tmp2, tmp3;
	tmp = a & b;
	tmp2 = a & c;
	tmp3 = b & c;
	return tmp ^ tmp2 ^ tmp3;
}

void calculate_higher_values(uint32_t *w) {
	for (int j = 16; j < 64; j++) {
		uint32_t s0 = calc_s0(w[j-15]);
		uint32_t s1 = calc_s1(w[j-2]);
		w[j] = w[j-16] + s0 + w[j-7] + s1;
	}
}

void calculate_w_vector(uint32_t *w, char *input) {
	memcpy(w, input, 16*sizeof(uint32_t));
	calculate_higher_values(w);
}

void swap_bytes(char *input, char *output, size_t size) {
	size_t size_in_words = size / 4;
	for (size_t i = 0; i < size_in_words; i++) {
		uint32_t *input_32 = (uint32_t*)input+i;
		uint32_t *output_32 = (uint32_t*)output+i;

#if __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__
		*output_32 = *input_32;
#elif __BYTE_ORDER__ ==  __ORDER_LITTLE_ENDIAN__
		*output_32 = (*input_32 & 0xFF000000) >> 24 |
			(*input_32 & 0x00FF0000) >> 8 |
			(*input_32 & 0x0000FF00) << 8 |
			(*input_32 & 0x000000FF) << 24;
#else
#error "No endianess found"
#endif

	}
}


void calc_compression(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
		uint32_t *e, uint32_t *f, uint32_t *g, uint32_t *h, uint32_t *w,
		int i) {
	uint32_t S1 = calc_S1(*e);
	uint32_t ch = calc_ch(*e, *f, *g);
	uint32_t temp1 = *h + S1 + ch + k[i] + w[i];
	uint32_t S0 = calc_S0(*a);
	uint32_t maj = calc_maj(*a, *b, *c);
	uint32_t temp2 = S0 + maj;

	*h = *g;
	*g = *f;
	*f = *e;
	*e = *d + temp1;
	*d = *c;
	*c = *b;
	*b = *a;
	*a = temp1 + temp2;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
