#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if SHA_BITS == 256

typedef uint32_t base_type;

static base_type _h[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
static const base_type k[64] = {
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
static const size_t W_SIZE     = 64;
static const size_t BLOCK_SIZE = 64;

#elif SHA_BITS == 512
#else
#error "Invalid SHA_BITS"
#endif

/* Calculates padding size plus 8-byte length at the end */
void calculate_padded_msg_size(size_t size, size_t *res) {
	// direct way to calculate padding
	// 64 bytes == 512 bits
	*res = 0;
	// if 0x80 prepend plus 8 bytes for message length doesn't fit the next 64
	// bytes multiple, use another one to make sure it's padded to 64 bytes.
	if (size % 64 > 64-9) *res += 64;

	*res += 64 - (size % 64);
}
void calculate_padded_msg_size_FIPS_180_4(size_t size, size_t *res) {
	// analytical way to calculate padding
	size_t k = 0;
	while(1) {
		if (((k + 8*size + 1) % 512) == 448)
			break;
		k++;
	}
	if(((k+1)%8) != 0) {printf("ERROR\n"); exit(1);}
	*res = ((k+1)/8) + 8;
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

base_type calc_s1(base_type e) {
	base_type tmp, tmp2, tmp3;
	tmp = rotate_right(e, 17);
	tmp2 = rotate_right(e, 19);
	tmp3 = e >> 10;
	return tmp ^ tmp2 ^ tmp3;
}
base_type calc_S1(base_type e) {
	base_type tmp, tmp2, tmp3;
	tmp = rotate_right(e, 6);
	tmp2 = rotate_right(e, 11);
	tmp3 = rotate_right(e, 25);
	return tmp ^ tmp2 ^ tmp3;
}
base_type calc_ch(base_type e, base_type f, base_type g) {
	base_type tmp, tmp2;
	tmp = e & f;
	tmp2 = ~e & g;
	return tmp ^ tmp2;
}
base_type calc_S0(base_type a) {
	base_type tmp, tmp2, tmp3;
	tmp = rotate_right(a, 2);
	tmp2 = rotate_right(a, 13);
	tmp3 = rotate_right(a, 22);
	return tmp ^ tmp2 ^ tmp3;
}
base_type calc_s0(base_type a) {
	base_type tmp, tmp2, tmp3;
	tmp = rotate_right(a, 7);
	tmp2 = rotate_right(a, 18);
	tmp3 = a >> 3;
	return tmp ^ tmp2 ^ tmp3;
}
base_type calc_maj(base_type a, base_type b, base_type c) {
	base_type tmp, tmp2, tmp3;
	tmp = a & b;
	tmp2 = a & c;
	tmp3 = b & c;
	return tmp ^ tmp2 ^ tmp3;
}

void calculate_higher_values(base_type *w) {
	for (int j = 16; j < 64; j++) {
		base_type s0 = calc_s0(w[j-15]);
		base_type s1 = calc_s1(w[j-2]);
		w[j] = w[j-16] + s0 + w[j-7] + s1;
	}
}

void calculate_w_vector(base_type *w, char *input) {
	memcpy(w, input, 16*sizeof(base_type));
	calculate_higher_values(w);
}

void swap_bytes(char *input, char *output, size_t size) {
	size_t size_in_words = size / 4;
	for (size_t i = 0; i < size_in_words; i++) {
		base_type *input_cast = (base_type*)input+i;
		base_type *output_cast = (base_type*)output+i;

#if __BYTE_ORDER__ ==  __ORDER_LITTLE_ENDIAN__

		*output_cast =
#if SHA_BITS == 256
			(*input_cast & 0xFF000000) >> 24 |
			(*input_cast & 0x00FF0000) >>  8 |
			(*input_cast & 0x0000FF00) <<  8 |
			(*input_cast & 0x000000FF) << 24;
#elif SHA_BITS == 512
			(*input_cast & 0xFF00000000000000ULL) >> 56 |
			(*input_cast & 0x00FF000000000000ULL) >> 40 |
			(*input_cast & 0x0000FF0000000000ULL) >> 24 |
			(*input_cast & 0x000000FF00000000ULL) >>  8 |
			(*input_cast & 0x00000000FF000000ULL) <<  8 |
			(*input_cast & 0x0000000000FF0000ULL) << 24 |
			(*input_cast & 0x000000000000FF00ULL) << 40 |
			(*input_cast & 0x00000000000000FFULL) << 56;
#endif // SHA_BITS

#else
#error "Only for little endian"
#endif

	}
}


void calc_compression(base_type *a, base_type *b, base_type *c, base_type *d,
		base_type *e, base_type *f, base_type *g, base_type *h, base_type *w,
		int i) {
	base_type S1 = calc_S1(*e);
	base_type ch = calc_ch(*e, *f, *g);
	base_type temp1 = *h + S1 + ch + k[i] + w[i];
	base_type S0 = calc_S0(*a);
	base_type maj = calc_maj(*a, *b, *c);
	base_type temp2 = S0 + maj;

	*h = *g;
	*g = *f;
	*f = *e;
	*e = *d + temp1;
	*d = *c;
	*c = *b;
	*b = *a;
	*a = temp1 + temp2;
}

int sha2 (int argc, char *argv[]) {
	char *filename = argv[1];
	FILE *file = fopen(filename, "r");

	if (file == NULL) { printf("ERROR\n"); return 1; }

	/* Get Size of file */
	fseek(file, 0, SEEK_END); // seek to end of file
	size_t size = ftell(file); // get current file pointer
	fseek(file, 0, SEEK_SET); // seek back to beginning of file

	char input[size];

	/* Save file in buffer */
	if (fread(input, sizeof(char), size, file) != size) {
		printf("ERROR\n"); return 1;
	}

	/* Padding */
	size_t padded_size;
	calculate_padded_msg_size(size, &padded_size);
	char input_padded[padded_size+size];
	pad(input, input_padded, size, padded_size);

	/* Swap bytes due to endianess */
	char input_swapped[padded_size+size];
	swap_bytes(input_padded, input_swapped, padded_size+size);

	/* write total message size (64bits) as the last 2 words */
	write_size(input_swapped, size, padded_size+size-8);

	/* Sha compression process */
	for (int i = 0; i < size + padded_size; i = i + BLOCK_SIZE) {
		base_type w[W_SIZE];
		calculate_w_vector(w, input_swapped+i);
		base_type a, b, c, d, e, f, g, h;
		a = _h[0];
		b = _h[1];
		c = _h[2];
		d = _h[3];
		e = _h[4];
		f = _h[5];
		g = _h[6];
		h = _h[7];

		for (int j = 0; j < W_SIZE; j++) {
			calc_compression(&a, &b, &c ,&d ,&e ,&f ,&g ,&h, w, j);
		}

		_h[0] = _h[0]+a;
		_h[1] = _h[1]+b;
		_h[2] = _h[2]+c;
		_h[3] = _h[3]+d;
		_h[4] = _h[4]+e;
		_h[5] = _h[5]+f;
		_h[6] = _h[6]+g;
		_h[7] = _h[7]+h;

	}


	printf("padded size: %zu\n", padded_size);
	printf("input file size: %zu\n", size);
	printf(
#if SHA_BITS == 256
			"%08x%08x%08x%08x%08x%08x%08x%08x\n",
#elif SHA_BITS == 512
			"%016lx%016lx%016lx%016lx%016lx%016lx%016lx%016lx\n",
#endif
			_h[0],_h[1],_h[2],_h[3],_h[4],_h[5],_h[6],_h[7]);
	fclose(file);
	return 0;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
