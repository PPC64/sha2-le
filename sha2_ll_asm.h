#ifndef _PPC64_LE_SHA2_ASM_LL_H_
#define _PPC64_LE_SHA2_ASM_LL_H_

#if LOW_LEVEL != 2
#error "The sha2_ll_asm.h header should only be included on LOW_LEVEL == 2"
#endif

#include "base-types.h"

#define ROTR(n, b) (((n) >> (b)) | ((n) << ((base_type_size * 8) - (b))))

#define SHR(x, n) ((x) >> (n))

#define BIGSIGMA0(x) (ROTR((x), S0_args[0]) ^ ROTR((x), S0_args[1]) ^ \
  ROTR((x), S0_args[2]))

#define BIGSIGMA1(x) (ROTR((x), S1_args[0]) ^ ROTR((x), S1_args[1]) ^ \
  ROTR((x), S1_args[2]))

#define SIGMA0(x) (ROTR((x), s0_args[0]) ^ ROTR((x), s0_args[1]) ^ \
  SHR((x), s0_args[2]))

#define SIGMA1(x) (ROTR((x), s1_args[0]) ^ ROTR((x), s1_args[1]) ^ \
  SHR((x), s1_args[2]))


void sha2_transform(base_type* _h, base_type* w) {
  base_type a, b, c, d, e, f, g, h, tmp1, tmp2;

  a = _h[0];
  b = _h[1];
  c = _h[2];
  d = _h[3];
  e = _h[4];
  f = _h[5];
  g = _h[6];
  h = _h[7];


#if SHA_BITS == 256
  int Rb = 8, Rc = 4;
  int j = W_SIZE; // 64
  __asm__(
    "lvsl       7,0,%2\n\t"
    "lvsr       10,0,%3\n\t"
    "BEGIN_LOOP:\n\t"
    "add        27,%1,%0\n\t"   // alias to W[j] location
    "andi.      24,27,0xf\n\t"  // aligned in 16 bit?
    "beq LOAD_ALIGNED\n\t"
    //Assumption: On first iteration, the first element is aligned on 16 bits.
    "addi       26,23,-48\n\t"
    "lvx        4,0,26\n\t"

    "vperm      5,1,5,7\n\t"    // load 4 words to vector: w[j-7] to w[j-3]
                                // This operation reuses the v1 value loaded
                                // on last iteration.

    // Use previous result here.
    "vperm      1,8,1,7\n\t"    // load 4 words to vector: w[j-2] to w[j+1]
                                // This operation reuses the v1 value loaded
                                // on last iteration.
                                // v8 carries the last result calculated. Its
                                // content is merged with v1.

    "vperm      0,4,0,7\n\t"    // load 4 words to vector: w[j-14] to w[j-11]
                                // This operation reuses the v0 value loaded
                                // on last iteration.

    "vperm      9,8,8,7\n\t"    // Move previous result to high half


    "b MAIN\n\t"

    "LOAD_ALIGNED:\n\t"
    "mr         23,27\n\t"      // Save store location
    "addi       26,27,-64\n\t"
    "lvx        0,0,26\n\t"     // load 4 words to vector: w[j-16] to w[j-13]

    "addi       26,27,-16\n\t"
    "lvx        1,0,26\n\t"     // load 4 words to vector: w[j-4] to w[j-1]

    "addi       26,27,-32\n\t"
    "lvx        5,0,26\n\t"     // load 4 words to vector: w[j-8] to w[j-5]

    "MAIN:\n\t"
    "vperm      2,0,0,10\n\t"    // v2 = w[j-15], w[j-14], w[j-13], w[j-16]

    "vperm      3,1,1,7\n\t"    // v3 = w[j-2], W[j-1], W[j-4], W[j-3]

    "vshasigmaw 2,2,0,0\n\t"    // v2 = s0(w[j-15]),s0(w[j-14])
    "vshasigmaw 3,3,0,3\n\t"    // v3 = s1(w[j-2]) ,s1(w[j-1])

    "vmrglw     6,5,5\n\t"      // v6 = w[j-8], w[j-8], w[j-7], w[j-7]
    "vmrghw     6,5,6\n\t"      // v6 = w[j-7], w[j-6]. w[j-7], w[j-5]

    "vadduwm    4,2,3\n\t"      // v4 = s0(w[j-15])+s1(w[j-2]),s0(w[j-14])+s1(w[j-1]),...
    "vadduwm    8,6,0\n\t"      // v8 = w[j-7]+w[j-16],w[j-6]+w[j-15],...
    "vadduwm    8,8,4\n\t"      // v8 = v8+v4

    "beq END\n\t"             // Branch if current iteration is aligned

    "vperm      9,8,9,7\n\t"
    "stvx       9,0,23\n\t"   // store it in W[j-2] to W[j+2]
    "END:\n\t"
    "addi       %1,%1,8\n\t"
    "cmpi       0,1,%1,256\n\t"
    "blt BEGIN_LOOP"
    :
    :"r"(w), "r"(j), "r"(Rb), "r"(Rc)
    :"r23", "r24", "r26","r27","v0","v1","v2","v3","v4","v5","v6","v7","v8",
     "v9","v10","memory"
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


  for (int i = 0; i < W_SIZE; i++) {
    tmp1 = h + BIGSIGMA1(e) + Ch(e, f, g) + k[i] + w[i];
    tmp2 = BIGSIGMA0(a) + Maj(a, b, c);

    h = g;
    g = f;
    f = e;
    e = d + tmp1;
    d = c;
    c = b;
    b = a;
    a = tmp1 + tmp2;
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
