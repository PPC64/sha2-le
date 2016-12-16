#if LOW_LEVEL != 2
#error "The sha2_ll_asm.h header should only be included on LOW_LEVEL == 2"
#endif

void calculate_higher_values(base_type *w) {
	for (int j = 16; j < W_SIZE; j++) {
#if SHA_BITS == 256
		__asm__ volatile(
				"sldi       27,%1,2\n\t"    // j * 4 (word size)
				"add        27,27,%0\n\t"   // alias to W[j] location
				"lwz        28,-60(27)\n\t" // W[j-15]
				"stw        28,-16(1)\n\t"  // store it in order to be read by vector
				"lwz        28,-8(27)\n\t"  // W[j-2]
				"stw        28,-12(1)\n\t"  // store it in order to be read by vector
				"la         28,-16(1)\n\t"  // use r0 and -4(r1) as temporary
				"lvx        0,0,28\n\t"     // load 4 words to a vector
				"vshasigmaw 0,0,0,0x2\n\t"  // apply small sigma 0 function (only to 0x1 bit)
				"stvx       0,0,28\n\t"     // store back 4 words
				"lwz        28,-16(1)\n\t"  // load resulted word to return value
				"lwz        29,-12(1)\n\t"  // load resulted word to return value
				"add        29,29,28\n\t"   // s0 + s1
				"lwz        28,-64(27)\n\t" // W[j-16]
				"add        29,29,28\n\t"   // s0 + s1 + W[j-16]
				"lwz        28,-28(27)\n\t" // W[j-7]
				"add        29,29,28\n\t"   // s0 + s1 + W[j-7]
				"stw        29,0(27)\n\t"   // store it in W[j]
				:
				:"r"(w), "r"(j)
				:"r27","r28","r29","v0","memory"
	   );
#elif SHA_BITS == 512
		base_type s0,s1;
		__asm__ volatile(
				"la         0,-16(1)\n\t"   // use r0 and -16(r1) as temporary
				"std        %2,-16(1)\n\t"  // store it in order to be read by vector
				"std        %3,-8(1)\n\t"   // store it in order to be read by vector
				"lvx        0,0,0\n\t"      // load 2 doublewords to a vector
				"vshasigmad 0,0,0,0xD\n\t"  // apply small sigma 0 function (only to 2*i = 0x2 bit)
				"stvx       0,0,0\n\t"      // store back 2 doublewords
				"ld         %0,-16(1)\n\t"  // load resulted word to return value
				"ld         %1,-8(1)\n\t"   // load resulted word to return value
				:"=r"(s0),"=r"(s1)
				:"r"(w[j-15]),"r"(w[j-2])
				:"r0","v0","memory"
		   );
		w[j] = w[j-16] + s0 + w[j-7] + s1;
#endif
	}
}

void calc_compression(base_type *_h, base_type *w) {
	base_type a, b, c, d, e, f, g, h;
	a = _h[0];
	b = _h[1];
	c = _h[2];
	d = _h[3];
	e = _h[4];
	f = _h[5];
	g = _h[6];
	h = _h[7];

	for (int i = 0; i < W_SIZE; i++) {
#if SHA_BITS == 256
		// whole compression is in assembly
		__asm__ volatile(
				"la         5,-16(1)\n\t"  // use -16(r1) as temporary
				"stw        %0,0(5)\n\t"   // wanna be S0
				"stw        %4,4(5)\n\t"   // wanna be S1
				"lvx        0,0,5\n\t"     // load 4 words to a vector
				"vshasigmaw 0,0,1,0xE\n\t" // apply big sigma 0 function (only to 0x1 bit)
				"stvx       0,0,5\n\t"     // store back 4 words
				"lwz        6,0(5)\n\t"    // S0
				"lwz        7,4(5)\n\t"    // S1

				"andc       5,%6,%4\n\t"   // g & e
				"and        8,%4,%5\n\t"   // e & f
				"xor        5,5,8\n\t"     // ch = (g & e) ^ (e & f)
				"add        8,%16,%17\n\t" // k[0] + w[0]
				"add        8,8,7\n\t"     // (k[0] + w[0]) + S1(e)
				"add        5,5,8\n\t"     // ch + (k[0] + w[0]) + S1(e)
				"add        5,5,%7\n\t"    // temp1 = (ch + k[0] + w[0] + S1(e)) + h

				"xor        7,%1,%2\n\t"   // b ^ c
				"and        7,7,%0\n\t"    // (b ^ c) & a
				"and        8,%1,%2\n\t"   // b & c
				"xor        8,7,8\n\t"     // maj = ((b ^ c) & a) ^ (b & c)
				"add        6,8,6\n\t"     // temp2 = maj + S0(a)

				"mr         %7,%6\n\t"     // h = g
				"mr         %6,%5\n\t"     // g = f
				"mr         %5,%4\n\t"     // f = e
				"add        %4,5,%3\n\t"   // e = temp1 + d
				"clrldi     %4,%4,32\n\t"  // e (truncate to word)
				"mr         %3,%2\n\t"     // d = c
				"mr         %2,%1\n\t"     // c = b
				"mr         %1,%0\n\t"     // b = a
				"add        %0,5,6\n\t"    // a = temp1 + temp2
				"clrldi     %0,%0,32\n\t"  // a (truncate to word)

				:"=r"(a),"=r"(b),"=r"(c),"=r"(d),"=r"(e),"=r"(f),"=r"(g),"=r"(h)
				: "0"(a), "1"(b), "2"(c), "3"(d), "4"(e), "5"(f), "6"(g), "7"(h),
				 "r"(w[i]),"r"(k[i])
				:"r5","r6","r7","r8","v0","memory"
			   );
#elif SHA_BITS == 512
		// only core sigma functions are in assembly
		base_type S1, S0;
		__asm__ volatile(
				"la         0,-16(1)\n\t"   // use r0 and -16(r1) as temporary
				"std        %2,-16(1)\n\t"  // store it in order to be read by vector
				"std        %3,-8(1)\n\t"
				"lvx        0,0,0\n\t"      // load 2 doublewords to a vector
				"vshasigmad 0,0,1,0xD\n\t"  // apply big sigma 0 function (only to 2*i = 0x2 bit)
				"stvx       0,0,0\n\t"      // store back 2 doublewords
				"ld         %0,-16(1)\n\t"  // load resulted word to return value
				"ld         %1,-8(1)\n\t"   // load resulted word to return value
				:"=r"(S0),"=r"(S1)
				:"r"(a),"r"(e)
				:"r0","v0","memory"
			   );
		base_type ch = calc_ch(e, f, g);
		base_type temp1 = h + S1 + ch + k[i] + w[i];
		base_type maj = calc_maj(a, b, c);
		base_type temp2 = S0 + maj;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
#endif // SHA_BITS
	}
	_h[0] += a;
	_h[1] += b;
	_h[2] += c;
	_h[3] += d;
	_h[4] += e;
	_h[5] += f;
	_h[6] += g;
	_h[7] += h;
}

// if using vim with "set modeline" on your .vimrc, then this file will be
// automatically configured as:
// vim: noai:ts=4:sw=4:sts=4:noet :
