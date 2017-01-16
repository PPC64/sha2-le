#ifndef _PPC64_LE_SHA2_NO_LL_H_
#define _PPC64_LE_SHA2_NO_LL_H_

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

void static inline sha2_round(base_type* a, base_type* b, base_type* c,
                              base_type* d, base_type* e, base_type* f,
                              base_type* g, base_type* h, base_type kplusw) {

#if SHA_BITS == 256

  base_type _a = *a;
  base_type _b = *b;
  base_type _c = *c;
  base_type _d = *d;
  base_type _e = *e;
  base_type _f = *f;
  base_type _g = *g;
  base_type _h = *h;

  __asm__ volatile (
#if 1 // instruction scheduled version - performance is better
    "rotlwi  10,%[e],26\n\t"    // r10 = ROTR(e, 6)
    "rotlwi  8,%[e],21\n\t"     // r8  = ROTR(e, 11)
    "and     7,%[f],%[e]\n\t"   // r7  = e & f
    "xor     8,10,8\n\t"        // r8  = ROTR(e, 6) ^ ROTR(e, 11)
    "andc    9,%[g],%[e]\n\t"   // r9  = !e & g
    "rotlwi  10,%[e],7\n\t"     // r10 = ROTR(e, 25)
    "rotlwi  6,%[a],30\n\t"     // r6  = ROTR(a, 2)
    "xor     10,8,10\n\t"       // r10 = S1(e)
    "xor     9,9,7\n\t"         // r9  = Ch(e, f, g)
    "xor     23,%[c],%[b]\n\t"  // r23 = c ^ b
    "rotlwi  8,%[a],19\n\t"     // r8  = ROTR(a, 13)
    "add     9,10,9\n\t"        // r9  = S1(e) + Ch(e, f, g)
    "xor     7,6,8\n\t"         // r8  = ROTR(a, 2) ^ ROTR(a, 13)
    "and     6,23,%[a]\n\t"     // r6  = (c ^ b) & a
    "rotlwi  8,%[a],10\n\t"     // r8  = ROTR(a, 22)
    "and     10,%[b],%[c]\n\t"  // r10 = b & c
    "xor     8,7,8\n\t"         // r8  = S0(a)
    "xor     10,6,10\n\t"       // r10 = Maj(a,b,c)
    "add     9,9,%[kpw]\n\t"    // r9  = S1(e) + Ch(e, f, g) + K[j] + W[j]
    "add     10,8,10\n\t"       // r10 = T2 = S0(a) + Maj(a,b,c)
    "add     9,9,%[h]\n\t"      // r9  = T1
    "mr      %[h],%[g]\n\t"     // h'  = g
    "add     8,%[d],9\n\t"      // r8 = d + T1
    "mr      %[g],%[f]\n\t"     // g'  = f
    "mr      %[f],%[e]\n\t"     // f'  = e
    "mr      %[d],%[c]\n\t"     // d'  = c
    "mr      %[c],%[b]\n\t"     // c'  = b
    "clrldi  %[e],8,32\n\t"     // e'  = (base_type)(d + T1)
    "add     9,10,9\n\t"        // r9  = T2 + T1
    "mr      %[b],%[a]\n\t"     // b'  = a
    "clrldi  %[a],9,32\n\t"     // a'  = (base_type)(T2 + T1)
#else // easy to read version - performance is worse
    "rotlwi  10,%[e],26\n\t"    // r10 = ROTR(e, 6)
    "rotlwi  8,%[e],21\n\t"     // r8  = ROTR(e, 11)
    "xor     8,10,8\n\t"        // r8  = ROTR(e, 6) ^ ROTR(e, 11)
    "rotlwi  10,%[e],7\n\t"     // r10 = ROTR(e, 25)
    "xor     10,8,10\n\t"       // r10 = S1(e)

    "andc    9,%[g],%[e]\n\t"   // r9  = !e & g
    "and     7,%[f],%[e]\n\t"   // r7  = e & f
    "xor     9,9,7\n\t"         // r9  = Ch(e, f, g)

    "add     9,10,9\n\t"        // r9  = S1(e) + Ch(e, f, g)
    "add     9,9,%[kpw]\n\t"    // r9  = S1(e) + Ch(e, f, g) + K[j] + W[j]
    "add     9,9,%[h]\n\t"      // r9  = T1 = h + S1(e) + Ch(e, f, g) + ...

    "rotlwi  6,%[a],30\n\t"     // r6  = ROTR(a, 2)
    "rotlwi  8,%[a],19\n\t"     // r8  = ROTR(a, 13)
    "xor     7,6,8\n\t"         // r8  = ROTR(a, 2) ^ ROTR(a, 13)
    "rotlwi  8,%[a],10\n\t"     // r8  = ROTR(a, 22)
    "xor     8,7,8\n\t"         // r8  = S0(a)

    "xor     23,%[c],%[b]\n\t"  // r23 = c ^ b
    "and     6,23,%[a]\n\t"     // r6  = (c ^ b) & a
    "and     10,%[b],%[c]\n\t"  // r10 = b & c
    "xor     10,6,10\n\t"       // r10 = Maj(a,b,c)

    "add     10,8,10\n\t"       // r10 = T2 = S0(a) + Maj(a,b,c)
    "add     8,%[d],9\n\t"      // r8 = d + T1
    "add     9,10,9\n\t"        // r9  = T2 + T1

    "mr      %[h],%[g]\n\t"     // h'  = g
    "mr      %[g],%[f]\n\t"     // g'  = f
    "mr      %[f],%[e]\n\t"     // f'  = e
    "clrldi  %[e],8,32\n\t"     // e'  = (base_type)(d + T1)
    "mr      %[d],%[c]\n\t"     // d'  = c
    "mr      %[c],%[b]\n\t"     // c'  = b
    "mr      %[b],%[a]\n\t"     // b'  = a
    "clrldi  %[a],9,32\n\t"     // a'  = (base_type)(T2 + T1)
#endif
    : // output list
      [a] "+r" (_a),
      [b] "+r" (_b),
      [c] "+r" (_c),
      [d] "+r" (_d),
      [e] "+r" (_e),
      [f] "+r" (_f),
      [g] "+r" (_g),
      [h] "+r" (_h)
    : // input list
      [kpw] "r" (kplusw)
    : // clobber list
      "r6", "r7", "r8", "r9", "r10", "r23"
  );

  *a = _a;
  *b = _b;
  *c = _c;
  *d = _d;
  *e = _e;
  *f = _f;
  *g = _g;
  *h = _h;

#else // SHA_BITS == 512

  base_type tmp1, tmp2;

  tmp1 = *h + BIGSIGMA1(*e) + Ch(*e, *f, *g) + kplusw;
  tmp2 = BIGSIGMA0(*a) + Maj(*a, *b, *c);

  *h = *g;
  *g = *f;
  *f = *e;
  *e = *d + tmp1;
  *d = *c;
  *c = *b;
  *b = *a;
  *a = tmp1 + tmp2;

#endif

}

void sha2_transform(base_type* _h, base_type* w) {
  base_type a, b, c, d, e, f, g, h;
  vector_base_type kplusw;
  int Rb = 8;
  vector int vRb;
  int j;

  a = _h[0];
  b = _h[1];
  c = _h[2];
  d = _h[3];
  e = _h[4];
  f = _h[5];
  g = _h[6];
  h = _h[7];

  // Loop unrolling, from 0 to 15
  for (j = 0; j < 16; j++) {
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[j] + w[j]);
  }

#if SHA_BITS == 256

  vector_base_type w0, w1, w2, w3;

  int Rc = 4;
  vector int vRc;

  // Load 16 elements from w out of the loop
  __asm__ volatile (
    "lvsl       %[vrb],0,%[rb]\n\t"   // parameter for vperm
    "lvsr       %[vrc],0,%[rc]\n\t"   // parameter for vperm
    "sldi       27,%[index],2\n\t"    // j * 4 (word size)
    "add        27,27,%[wptr]\n\t"    // alias to W[j] location

    // load 4 words to vector: w[j-16] to w[j-13]
    "addi       26,27,-64\n\t"
    "lvx        %[w0],0,26\n\t"

    // load 4 words to vector: w[j-12] to w[j-9]
    "addi       26,27,-48\n\t"
    "lvx        %[w1],0,26\n\t"

    // load 4 words to vector: w[j-8] to w[j-5]
    "addi       26,27,-32\n\t"
    "lvx        %[w2],0,26\n\t"

    // load 4 words to vector: w[j-4] to w[j-1]
    "addi       26,27,-16\n\t"
    "lvx        %[w3],0,26\n\t"

    : // output list
      [w0] "=v" (w0),
      [w1] "=v" (w1),
      [w2] "=v" (w2),
      [w3] "=v" (w3),
      [vrb] "=v" (vRb),
      [vrc] "=v" (vRc)
    : // input list
      [index] "r" (j),
      [rb] "r" (Rb),
      [rc] "r" (Rc),
      [wptr] "r" (w)
    : // clobber list
      "r26", "r27", "memory"
  );

  // From 16 to W_SIZE (64)
  for (; j < W_SIZE; j = j + 4) {
    __asm__ volatile (
      "sldi       27,%[index],2\n\t"        // j * 4 (word size)

      "add        26,27,%[kptr]\n\t"        // alias to k[j] location
      "lvx        11,0,26\n\t"

      // v4 = w[j-15], w[j-14], w[j-13], w[j-12]
      "vperm      4,%[w1],%[w0],%[vrc]\n\t"

      // v12 = w[j-7], w[j-6], w[j-5], w[j-4]
      "vperm      12,%[w3],%[w2],%[vrc]\n\t"

      // v13 = w[j-2], w[j-1], w[j-4], w[j-3]
      "vperm      13,%[w3],%[w3],%[vrb]\n\t"

      // v4 = s0(w[j-15]) , s0(w[j-14]) , s0(w[j-13]) , s0(w[j-12])
      "vshasigmaw 4,4,0,0\n\t"

      // v5 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j-4]) , s1(w[j-3])
      "vshasigmaw 5,13,0,0xf\n\t"

      // v6 = s0(w[j-15]) + w[j-7],
      //      s0(w[j-14]) + w[j-6],
      //      s0(w[j-13]) + w[j-5],
      //      s0(w[j-12]) + w[j-4]
      "vadduwm    6,4,12\n\t"
      // v8 = s0(w[j-15]) + w[j-7] + w[j-16],
      //      s0(w[j-14]) + w[j-6] + w[j-15],
      //      s0(w[j-13]) + w[j-5] + w[j-14],
      //      s0(w[j-12]) + w[j-4] + w[j-13]
      "vadduwm    8,6,%[w0]\n\t"
      // v9 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]
      //      s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]
      //      s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j-4]), // UNDEFINED
      //      s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j-3])  // UNDEFINED
      "vadduwm    9,8,5\n\t"

      // At this point, v9[0] and v9[1] are the correct values to be stored at
      // w[j] and w[j+1].
      // v9[2] and v9[3] are not considered

      // v3 = s1(w[j]) , s1(s(w[j+1]) , UNDEFINED , UNDEFINED
      "vshasigmaw 3,9,0,0xf\n\t"

      // TODO: review this!! There must be a more efficient way.
      // v5 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j]) , s1(w[j+1])
      "vperm      3,3,3,%[vrb]\n\t"
      "vperm      5,5,3,%[vrb]\n\t"
      "vperm      5,5,5,%[vrb]\n\t"

      // v9 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]
      //      s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]
      //      s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j]),   // w[j+2]
      //      s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j+1])  // w[j+4]
      "vadduwm    9,8,5\n\t"

      // Updating w0 to w3 to hold the "new previous" 16 values from w.
      "vor        %[w0_out],%[w1],%[w1]\n\t"
      "vor        %[w1_out],%[w2],%[w2]\n\t"
      "vor        %[w2_out],%[w3],%[w3]\n\t"
      "vor        %[w3_out],9,9\n\t"

      // store k + w to kplusw (4 values at once)
      "vadduwm    %[kplusw],9,11\n\t"

      : // output list
        [w0_out] "=v" (w0),
        [w1_out] "=v" (w1),
        [w2_out] "=v" (w2),
        [w3_out] "=v" (w3),
        [kplusw] "=v" (kplusw)
      : // input list
        [index] "r" (j),
        [vrb] "v" (vRb),
        [vrc] "v" (vRc),
        [kptr] "r" (k),
        [wptr] "r" (w),
        [w0] "[w0_out]" (w0),
        [w1] "[w1_out]" (w1),
        [w2] "[w2_out]" (w2),
        [w3] "[w3_out]" (w3)
      : // clobber list
        "v3", "v4", "v5", "v6", "v8", "v9", "v11", "v12", "v13", "r26", "r27",
        "memory"
    );
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[0]);
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[1]);
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[2]);
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[3]);
  }

#else // SHA_BITS == 512

  vector_base_type w0, w1, w2, w3, w4, w5, w6, w7;

  // Load 16 elements from w out of the loop
  __asm__ volatile(
    "lvsl       %[vrb],0,%[rb]\n\t"   // parameter for vperm
    "sldi       27,%[index],3\n\t"    // j * 8 (double word size)
    "add        27,27,%[wptr]\n\t"    // alias to W[j] location

    // load 2 doublewords to vector: w[j-16] and w[j-15]
    "addi       26,27,-128\n\t"
    "lvx        %[w0],0,26\n\t"

    // load 2 doublewords to vector: w[j-14] and w[j-13]
    "addi       26,27,-112\n\t"
    "lvx        %[w1],0,26\n\t"

    // load 2 doublewords to vector: w[j-12] and w[j-11]
    "addi       26,27,-96\n\t"
    "lvx        %[w2],0,26\n\t"

    // load 2 doublewords to vector: w[j-10] and w[j-9]
    "addi       26,27,-80\n\t"
    "lvx        %[w3],0,26\n\t"

    // load 2 doublewords to vector: w[j-8] and w[j-7]
    "addi       26,27,-64\n\t"
    "lvx        %[w4],0,26\n\t"

    // load 2 doublewords to vector: w[j-6] and w[j-5]
    "addi       26,27,-48\n\t"
    "lvx        %[w5],0,26\n\t"

    // load 2 doublewords to vector: w[j-4] and w[j-3]
    "addi       26,27,-32\n\t"
    "lvx        %[w6],0,26\n\t"

    // load 4 words to vector: w[j-2] to w[j-1]
    "addi       26,27,-16\n\t"
    "lvx        %[w7],0,26\n\t"

    : // output list
      [vrb] "=v" (vRb),
      [w0] "=v" (w0),
      [w1] "=v" (w1),
      [w2] "=v" (w2),
      [w3] "=v" (w3),
      [w4] "=v" (w4),
      [w5] "=v" (w5),
      [w6] "=v" (w6),
      [w7] "=v" (w7)
    : // input list
      [index] "r" (j),
      [rb] "r" (Rb),
      [wptr] "r" (w)
    : // clobber list
      "memory", "r26", "r27"
  );

  // From 16 to W_SIZE (64)
  for (; j < W_SIZE; j = j + 2) {
    __asm__ volatile (
      "sldi       27,%[index],3\n\t"          // j * 8 (doubleword size)
      "add        26,27,%[kptr]\n\t"          // alias to k[j] location
      "lvx        11,0,26\n\t"

      "vperm      9,%[w1],%[w0],%[vrb]\n\t"   // v9 = w[j-15] , w[j-14]
      "vperm      10,%[w5],%[w4],%[vrb]\n\t"  // v10 = w[j-7] , w[j-6]

      "vshasigmad 9,9,0,0\n\t"                // v9 = s0(w[j-15]) , s0(w[j-14])
      "vshasigmad 12,%[w7],0,0xf\n\t"         // v12 = s1(w[j-2]) , s1(w[j-1])

      // v9 = s0(w[j-15]) + w[j-7] , s0(w[j-14]) + w[j-6]
      "vaddudm    9,9,10\n\t"
      // v8 = s1(w[j-2]) + w[j-16] , s1(w[j-1]) + w[j-15]
      "vaddudm    8,12,%[w0]\n\t"
      // v9 = s0(w[j-15]) + w[j-7] + s1(w[j-2]) + w[j-16],  // w[j]
      //      s0(w[j-14]) + w[j-6] + s1(w[j-1]) + w[j-15]   // w[j+1]
      "vaddudm    9,9,8\n\t"

      // Updating w0 to w7 to hold the "new previous" 16 values from w.
      "vor        %[w0_out],%[w1],%[w1]\n\t"
      "vor        %[w1_out],%[w2],%[w2]\n\t"
      "vor        %[w2_out],%[w3],%[w3]\n\t"
      "vor        %[w3_out],%[w4],%[w4]\n\t"
      "vor        %[w4_out],%[w5],%[w5]\n\t"
      "vor        %[w5_out],%[w6],%[w6]\n\t"
      "vor        %[w6_out],%[w7],%[w7]\n\t"
      "vor        %[w7_out],9,9\n\t"

      // store k + w to kplusw (2 values at once)
      "vaddudm    %[kplusw],9,11\n\t"

      : // output list
        [w0_out] "=v" (w0),
        [w1_out] "=v" (w1),
        [w2_out] "=v" (w2),
        [w3_out] "=v" (w3),
        [w4_out] "=v" (w4),
        [w5_out] "=v" (w5),
        [w6_out] "=v" (w6),
        [w7_out] "=v" (w7),
        [kplusw] "=v" (kplusw)
      : // input list
        [index] "r" (j),
        [kptr] "r" (k),
        [vrb] "v" (vRb),
        [w0] "[w0_out]" (w0),
        [w1] "[w1_out]" (w1),
        [w2] "[w2_out]" (w2),
        [w3] "[w3_out]" (w3),
        [w4] "[w4_out]" (w4),
        [w5] "[w5_out]" (w5),
        [w6] "[w6_out]" (w6),
        [w7] "[w7_out]" (w7)
      : // clobber list
        "r26", "r27", "v8", "v9", "v10", "v11", "v12", "memory"
    );

    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[0]);
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, kplusw[1]);
  }
#endif

  _h[0] += a;
  _h[1] += b;
  _h[2] += c;
  _h[3] += d;
  _h[4] += e;
  _h[5] += f;
  _h[6] += g;
  _h[7] += h;
}

#endif // _PPC64_LE_SHA2_NO_LL_H_
