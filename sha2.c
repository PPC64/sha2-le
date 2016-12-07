#include "sha2.h"

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
	for (int i = 0; i < size + padded_size; i = i + 64) { // 512 bits each piece
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


	printf("padded size: %d\n", (int)padded_size);
	printf("input file size: %d\n", (int)size);
	printf("%08x%08x%08x%08x%08x%08x%08x%08x\n",
			_h[0],_h[1],_h[2],_h[3],_h[4],_h[5],_h[6],_h[7]);
	fclose(file);
	return 0;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
