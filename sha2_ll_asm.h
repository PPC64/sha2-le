#ifndef _PPC64_LE_SHA2_ASM_LL_H_
#define _PPC64_LE_SHA2_ASM_LL_H_

#if LOW_LEVEL != 2
#error "The sha2_ll_asm.h header should only be included on LOW_LEVEL == 2"
#endif

void calculate_higher_values(base_type *w) {

#if SHA_BITS == 256
  int Rb = 8;
  int j = W_SIZE; // 64
  __asm__(
    "BEGIN_LOOP:\n\t"
    "add        27,%1,%0\n\t"   // alias to W[j] location
    "andi.      24,27,0xf\n\t"  // aligned in 16 bit?
    "beq LOAD_ALIGNED\n\t"
    //On first iteration, the first element is aligned on 16 bits.
    "addi       26,27,-64\n\t"
    "lvx        2,0,26\n\t"
    "lvsl       3,0,%2\n\t"
    "addi       26,27,-48\n\t"
    "lvx        4,0,26\n\t"
    "vperm      0,4,2,3\n\t"    // load 4 words to vector: W[j-16] to W[j-13]

    "addi       24,27,-16\n\t"
    "lvx        5,0,24\n\t"
    // Use previous result here.
    "vperm      1,8,5,3\n\t"    // load 4 words to vector: W[j-4] to W[j-1]

    "lvsr       3,0,%2\n\t"     // Move previous result to high half
    "vperm      9,8,8,3\n\t"

    "addi       26,27,-32\n\t"
    "lvx        2,0,26\n\t"
    "vperm      5,5,2,3\n\t"    // load 4 words to vector: W[j-32] to W[j-16]

    "b MAIN\n\t"

    "LOAD_ALIGNED:\n\t"
    "mr         23,27\n\t"      // Save store location
    "addi       26,27,-64\n\t"
    "lvx        0,0,26\n\t"     // load 4 words to vector: W[j-16] to W[j-13]

    "addi       26,27,-16\n\t"
    "lvx        1,0,26\n\t"     // load 4 words to vector: W[j-4] to W[j-1]

    "addi       26,27,-32\n\t"
    "lvx        5,0,26\n\t"     // load 4 words to vector: W[j-32] to W[j-16]

    "MAIN:\n\t"
    "vmrghw     2,0,0\n\t"      // v2 = W[j-14], W[j-14], W[j-13], W[j-13]
    "vmrglw     2,2,0\n\t"      // v2 = W[j-16], W[j-14], W[j-15], W[j-14]
    "vmrghw     2,1,2\n\t"      // v2 = W[j-15], W[j-2], W[j-14], W[j-1]
    "vshasigmaw 2,2,0,0xA\n\t"  // small sigma 0 (only to 0x1 bit)

    "vmrghw     7,2,2\n\t"      // v7 = W[j-14], W[j-14], W[j-1], W[j-1]
    "vmrgow     3,7,2\n\t"      // v3 = W[j-15], W[j-14] ...

    "vmrglw     4,2,2\n\t"
    "vmrghw     4,7,4\n\t"      // v4 = W[j-2], W[j-1] ...

    "vadduwm    4,4,3\n\t"
    "vadduwm    4,4,0\n\t"

    "vmrglw     6,5,5\n\t"
    "vmrghw     6,5,6\n\t"

    "vadduwm    8,4,6\n\t"

    "beq END\n\t"             // Branch if current iteration is aligned

    "lvsl       3,0,%2\n\t"
    "vperm      9,8,9,3\n\t"
    "stvx       9,0,23\n\t"   // store it in W[j-2] to W[j+2]
    "END:\n\t"
    "addi       %1,%1,8\n\t"
    "cmpi       0,1,%1,256\n\t"
    "blt BEGIN_LOOP"
    :
    :"r"(w), "r"(j), "r"(Rb)
    :"r23", "r24", "r26","r27","v0","v1","v2","v3","v4","v5","v6","v7","v8",
     "v9", "memory"
  );
#elif SHA_BITS == 512
  base_type s0,s1;

  for (int j = 16; j < W_SIZE; j++) {
    __asm__ volatile(
      "la         0,-16(1)\n\t"   // use r0 and -16(r1) as temporary
      "std        %2,-16(1)\n\t"  // store it in order to be read by vector
      "std        %3,-8(1)\n\t"   // store it in order to be read by vector
      "lvx        0,0,0\n\t"      // load 2 doublewords to a vector
      "vshasigmad 0,0,0,0xD\n\t"  // small sigma 0 (only to 2*i = 0x2 bit)
      "stvx       0,0,0\n\t"      // store back 2 doublewords
      "ld         %0,-16(1)\n\t"  // load resulted word to return value
      "ld         %1,-8(1)\n\t"   // load resulted word to return value
      :"=r"(s0),"=r"(s1)
      :"r"(w[j-15]),"r"(w[j-2])
      :"r0","v0","memory"
    );
    w[j] = w[j-16] + s0 + w[j-7] + s1;
  }
#endif
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
      "vshasigmaw 0,0,1,0xE\n\t" // big sigma 0 (only to 0x1 bit)
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
    base_type S0, S1;

    __asm__ volatile(
      "la         0,-16(1)\n\t"   // use r0 and -16(r1) as temporary
      "std        %2,-16(1)\n\t"  // store it in order to be read by vector
      "std        %3,-8(1)\n\t"
      "lvx        0,0,0\n\t"      // load 2 doublewords to a vector
      "vshasigmad 0,0,1,0xD\n\t"  // big sigma 0 (only to 2*i = 0x2 bit)
      "stvx       0,0,0\n\t"      // store back 2 doublewords
      "ld         %0,-16(1)\n\t"  // load resulted word to return value
      "ld         %1,-8(1)\n\t"   // load resulted word to return value
      :"=r"(S0),"=r"(S1)
      :"r"(a),"r"(e)
      :"r0","v0","memory"
    );
    base_type ch = (e & f) ^ (~e & g);
    base_type temp1 = h + S1 + ch + k[i] + w[i];
    base_type maj = (a & b) ^ (a & c) ^ (b & c);
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

#endif // _PPC64_LE_SHA2_ASM_LL_H_
