
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t _h[8] = {
0x6a09e667,
0xbb67ae85,
0x3c6ef372,
0xa54ff53a,
0x510e527f,
0x9b05688c,
0x1f83d9ab,
0x5be0cd19
};

uint32_t k[64] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void calculate_padded_msg_size(size_t size, size_t *res) {
	size_t k = 0;
	while(1) {
		if (((k + 8*size + 1) % 512) == 448)
			break;
		k++;
	}
	if(((k+1)%8) != 0) {printf("ERROR\n"); exit(1);}
	*res = (k+1)/8;
}

void pad(char *in, char *out, size_t size, size_t padded_size) {
	int i;
	for (i = 0; i < size; i ++)
		out[i] = in[i];
	out[size] = (char)(1); i++;//1? correct?

	for(; i < padded_size + size; i++)
		out[i] = 0;

}

void calculate_higher_values(uint32_t *w) {
  for (int j = 16; j < 64; j++) {
    uint32_t s0 = calc_S0(w[j-15]);
    uint32_t s1 = calc_S1(w[j-2]);
    w[j] = w[j-16] + s0 + w[j-7] + s1;
  }
}

void calculate_w_vector(uint32_t *w, char *input) {
	memcpy(w, input, 16*sizeof(uint32_t));
	calculate_higher_values(w);
}

uint32_t calc_S1(uint32_t e) {
	return 0;//TODO
}
uint32_t calc_ch(uint32_t e, uint32_t f, uint32_t g) {
	return 0;//TODO
}
uint32_t calc_S0(uint32_t a) {
	return 0;//TODO
}
uint32_t calc_maj(uint32_t a, uint32_t b, uint32_t c) {
	return 0;//TODO
}

void calc_compression(uint32_t *a,uint32_t *b,uint32_t *c ,uint32_t *d ,uint32_t *e ,uint32_t *f ,uint32_t *g ,uint32_t *h,
			uint32_t *w, int i) {
	uint32_t S1 = calc_S1(*e);
	uint32_t ch = calc_ch(*e, *f, *g);
	uint32_t temp1 = *h + S1 + ch + k[i] + w[i];
	uint32_t S0 = calc_S0(*a);
	uint32_t maj = calc_maj(*a,*b,*c);
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


int main (int argc, char *argv[]) {
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
	size_t  padded_size;
	calculate_padded_msg_size(size, &padded_size);
	char input_padded[padded_size+size];
	pad(input, input_padded, size, padded_size);

	/* Sha compression process */
	for (int i = 0; i < size + padded_size; i = i + 64) { //512 bits each piece
		uint32_t w[64];
		calculate_w_vector(w, input_padded+i);
		uint32_t a, b, c, d, e, f, g, h;
		a = _h[0];
		b = _h[1];
		c = _h[2];
		d = _h[3];
		e = _h[4];
		f = _h[5];
		g = _h[6];
		h = _h[7];
	
		for (int j = 0; j < 64; j++) {
			//uint32_t _a, _b, _c , _d , _e, _f, _g, _h;
			calc_compression(&a, &b, &c ,&d ,&e ,&f ,&g ,&h,  w, j);
			_h[0] = _h[0]+a;
			_h[1] = _h[1]+b;
			_h[2] = _h[2]+c;
			_h[3] = _h[3]+d;
			_h[4] = _h[4]+e;
			_h[5] = _h[5]+f;
			_h[6] = _h[6]+g;
			_h[7] = _h[7]+h;
		}
	
	}


	printf("padded size: %d\n", (int)padded_size);
	printf("input file size: %d\n", (int)size);
	printf("%x%x%x%x%x%x%x%x\n",_h[0],_h[1],_h[2],_h[3],_h[4],_h[5],_h[6],_h[7]);
	fclose(file);	
	return 0;
}
