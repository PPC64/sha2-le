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


/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */

#if SHA_BITS == 256

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {              \
  __asm__ volatile (                                                          \
    "rotlwi  10,%[e],26\n\t"    /* r10 = ROTR(e, 6)                        */ \
    "rotlwi  8,%[e],21\n\t"     /* r8  = ROTR(e, 11)                       */ \
    "and     7,%[f],%[e]\n\t"   /* r7  = e & f                             */ \
    "xor     8,10,8\n\t"        /* r8  = ROTR(e, 6) ^ ROTR(e, 11)          */ \
    "andc    9,%[g],%[e]\n\t"   /* r9  = !e & g                            */ \
    "rotlwi  10,%[e],7\n\t"     /* r10 = ROTR(e, 25)                       */ \
    "rotlwi  6,%[a],30\n\t"     /* r6  = ROTR(a, 2)                        */ \
    "xor     10,8,10\n\t"       /* r10 = S1(e)                             */ \
    "xor     9,9,7\n\t"         /* r9  = Ch(e, f, g)                       */ \
    "xor     23,%[c],%[b]\n\t"  /* r23 = c ^ b                             */ \
    "rotlwi  8,%[a],19\n\t"     /* r8  = ROTR(a, 13)                       */ \
    "add     9,10,9\n\t"        /* r9  = S1(e) + Ch(e, f, g)               */ \
    "xor     7,6,8\n\t"         /* r8  = ROTR(a, 2) ^ ROTR(a, 13)          */ \
    "and     6,23,%[a]\n\t"     /* r6  = (c ^ b) & a                       */ \
    "rotlwi  8,%[a],10\n\t"     /* r8  = ROTR(a, 22)                       */ \
    "and     10,%[b],%[c]\n\t"  /* r10 = b & c                             */ \
    "xor     8,7,8\n\t"         /* r8  = S0(a)                             */ \
    "xor     10,6,10\n\t"       /* r10 = Maj(a,b,c)                        */ \
    "add     9,9,%[kpw]\n\t"    /* r9  = S1(e) + Ch(e, f, g) + K[j] + W[j] */ \
    "add     10,8,10\n\t"       /* r10 = T2 = S0(a) + Maj(a,b,c)           */ \
    "add     9,9,%[h]\n\t"      /* r9  = T1                                */ \
    "add     8,%[d],9\n\t"      /* r8 = d + T1                             */ \
    "clrldi  %[d],8,32\n\t"     /* d'  = (base_type)(d + T1)               */ \
    "add     9,10,9\n\t"        /* r9  = T2 + T1                           */ \
    "clrldi  %[h],9,32\n\t"     /* h'  = (base_type)(T2 + T1)              */ \
    : /* output list */                                                       \
      [d] "+r" ((_d)),                                                        \
      [h] "+r" ((_h))                                                         \
    : /* input list */                                                        \
      [a] "r" ((_a)),                                                         \
      [b] "r" ((_b)),                                                         \
      [c] "r" ((_c)),                                                         \
      [e] "r" ((_e)),                                                         \
      [f] "r" ((_f)),                                                         \
      [g] "r" ((_g)),                                                         \
      [kpw] "r" ((_kplusw))                                                   \
    : /* clobber list */                                                      \
      "r6", "r7", "r8", "r9", "r10", "r23"                                    \
  ); } while (0)

#define LOAD_W(_w0, _w1, _w2, _w3, _vRb, _vRc, _j, _Rb, _Rc, _w) do {         \
  __asm__ volatile (                                                          \
    "lvsl    %[vrb],0,%[rb]\n\t"   /* parameter for vperm                  */ \
    "lvsr    %[vrc],0,%[rc]\n\t"   /* parameter for vperm                  */ \
    "sldi    27,%[index],2\n\t"    /* j * 4 (word size)                    */ \
    "add     27,27,%[wptr]\n\t"    /* alias to W[j] location               */ \
    "addi    26,27,-64\n\t"                                                   \
    "lvx     %[w0],0,26\n\t"       /* load w[j-16] to w[j-13] to vector    */ \
    "addi    26,27,-48\n\t"                                                   \
    "lvx     %[w1],0,26\n\t"       /* load w[j-12] to w[j-9] to vector     */ \
    "addi    26,27,-32\n\t"                                                   \
    "lvx     %[w2],0,26\n\t"       /* load w[j-8] to w[j-5] to vector      */ \
    "addi    26,27,-16\n\t"                                                   \
    "lvx     %[w3],0,26\n\t"       /* load w[j-4] to w[j-1] to vector      */ \
    : /* output list */                                                       \
      [w0] "=v" ((_w0)),                                                      \
      [w1] "=v" ((_w1)),                                                      \
      [w2] "=v" ((_w2)),                                                      \
      [w3] "=v" ((_w3)),                                                      \
      [vrb] "=v" ((_vRb)),                                                    \
      [vrc] "=v" ((_vRc))                                                     \
    : /* input list */                                                        \
      [index] "r" ((_j)),                                                     \
      [rb] "r" ((_Rb)),                                                       \
      [rc] "r" ((_Rc)),                                                       \
      [wptr] "r" ((_w))                                                       \
    : /* clobber list */                                                      \
      "r26", "r27", "memory"                                                  \
  ); } while (0)

#define CALC_4W(_w0, _w1, _w2, _w3, _kpw0, _kpw1, _kpw2, _kpw3,               \
                _j, _vRb, _vRc, _k) do {                                      \
  __asm__ volatile (                                                          \
    "sldi       27,%[index],2\n\t"        /* j * 4 (word size)             */ \
    "add        26,27,%[kptr]\n\t"        /* alias to k[j] location        */ \
    "lvx        11,0,26\n\t"                                                  \
    /* v4 = w[j-15], w[j-14], w[j-13], w[j-12]                             */ \
    "vperm      4,%[w1],%[w0],%[vrc]\n\t"                                     \
    /* v12 = w[j-7], w[j-6], w[j-5], w[j-4]                                */ \
    "vperm      12,%[w3],%[w2],%[vrc]\n\t"                                    \
    /* v13 = w[j-2], w[j-1], w[j-4], w[j-3]                                */ \
    "vperm      13,%[w3],%[w3],%[vrb]\n\t"                                    \
    /* v4 = s0(w[j-15]) , s0(w[j-14]) , s0(w[j-13]) , s0(w[j-12])          */ \
    "vshasigmaw 4,4,0,0\n\t"                                                  \
    /* v5 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j-4]) , s1(w[j-3])              */ \
    "vshasigmaw 5,13,0,0xf\n\t"                                               \
    /* v6 = s0(w[j-15]) + w[j-7],                                          */ \
    /*      s0(w[j-14]) + w[j-6],                                          */ \
    /*      s0(w[j-13]) + w[j-5],                                          */ \
    /*      s0(w[j-12]) + w[j-4]                                           */ \
    "vadduwm    6,4,12\n\t"                                                   \
    /* v8 = s0(w[j-15]) + w[j-7] + w[j-16],                                */ \
    /*      s0(w[j-14]) + w[j-6] + w[j-15],                                */ \
    /*      s0(w[j-13]) + w[j-5] + w[j-14],                                */ \
    /*      s0(w[j-12]) + w[j-4] + w[j-13]                                 */ \
    "vadduwm    8,6,%[w0]\n\t"                                                \
    /* v9 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]           */ \
    /*      s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]         */ \
    /*      s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j-4]), // UNDEFINED      */ \
    /*      s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j-3])  // UNDEFINED      */ \
    "vadduwm    9,8,5\n\t"                                                    \
    /* At this point, v9[0] and v9[1] are the correct values to be         */ \
    /* stored at w[j] and w[j+1].                                          */ \
    /* v9[2] and v9[3] are not considered                                  */ \
    /* v3 = s1(w[j]) , s1(s(w[j+1]) , UNDEFINED , UNDEFINED                */ \
    "vshasigmaw 3,9,0,0xf\n\t"                                                \
    /* TODO: review this!! There must be a more efficient way.             */ \
    /* v5 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j]) , s1(w[j+1])                */ \
    "vperm      3,3,3,%[vrb]\n\t"                                             \
    "vperm      5,5,3,%[vrb]\n\t"                                             \
    "vperm      5,5,5,%[vrb]\n\t"                                             \
    /* v9 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]           */ \
    /*      s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]         */ \
    /*      s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j]),   // w[j+2]         */ \
    /*      s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j+1])  // w[j+4]         */ \
    "vadduwm    9,8,5\n\t"                                                    \
    /* Updating w0 to w3 to hold the "new previous" 16 values from w.      */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                        \
    "vor        %[w1],%[w2],%[w2]\n\t"                                        \
    "vor        %[w2],%[w3],%[w3]\n\t"                                        \
    "vor        %[w3],9,9\n\t"                                                \
    /* store k + w to v9 (4 values at once)                                */ \
    "vadduwm    9,9,11\n\t"                                                   \
    /* Move first doubleword in v9 to kpw1                                 */ \
    "mfvsrd     %[kpw2], 41\n\t"                                              \
    /* Move low word to kpw0                                               */ \
    "srdi       %[kpw3], %[kpw2], 32\n\t"                                     \
    /* Clear low word. Keep high word.                                     */ \
    "clrldi     %[kpw2], %[kpw2], 32\n\t"                                     \
    /* Move higher double word to low.                                     */ \
    "vperm      9,9,9,%[vrb]\n\t"                                             \
    /* Move first doubleword in v9 to kpw2                                 */ \
    "mfvsrd     %[kpw0], 41\n\t"                                              \
    /* Move low word to kpw2                                               */ \
    "srdi       %[kpw1], %[kpw0], 32\n\t"                                     \
    /* Clear low word. Keep high word.                                     */ \
    "clrldi     %[kpw2], %[kpw2], 32\n\t"                                     \
    : /* output list */                                                       \
      [w0] "+v" ((_w0)),                                                      \
      [w1] "+v" ((_w1)),                                                      \
      [w2] "+v" ((_w2)),                                                      \
      [w3] "+v" ((_w3)),                                                      \
      [kpw0] "=r" ((_kpw0)),                                                  \
      [kpw1] "=r" ((_kpw1)),                                                  \
      [kpw2] "=r" ((_kpw2)),                                                  \
      [kpw3] "=r" ((_kpw3))                                                   \
    : /* input list */                                                        \
      [index] "r" ((_j)),                                                     \
      [vrb] "v" ((_vRb)),                                                     \
      [vrc] "v" ((_vRc)),                                                     \
      [kptr] "r" ((_k))                                                       \
    : /* clobber list */                                                      \
      "v3", "v4", "v5", "v6", "v8", "v9", "v11", "v12", "v13", "r26", "r27",  \
      "memory"                                                                \
  ); } while (0)

#else // SHA_BITS == 512

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {              \
  __asm__ volatile (                                                          \
    "rotldi  10,%[e],50\n\t"    /* r10 = ROTR(e, 14)                       */ \
    "rotldi  8,%[e],46\n\t"     /* r8  = ROTR(e, 18)                       */ \
    "and     7,%[f],%[e]\n\t"   /* r7  = e & f                             */ \
    "xor     8,10,8\n\t"        /* r8  = ROTR(e, 14) ^ ROTR(e, 18)         */ \
    "andc    9,%[g],%[e]\n\t"   /* r9  = !e & g                            */ \
    "rotldi  10,%[e],23\n\t"    /* r10 = ROTR(e, 41)                       */ \
    "rotldi  6,%[a],36\n\t"     /* r6  = ROTR(a, 28)                       */ \
    "xor     10,8,10\n\t"       /* r10 = S1(e)                             */ \
    "xor     9,9,7\n\t"         /* r9  = Ch(e, f, g)                       */ \
    "xor     23,%[c],%[b]\n\t"  /* r23 = c ^ b                             */ \
    "rotldi  8,%[a],30\n\t"     /* r8  = ROTR(a, 34)                       */ \
    "add     9,10,9\n\t"        /* r9  = S1(e) + Ch(e, f, g)               */ \
    "xor     7,6,8\n\t"         /* r8  = ROTR(a, 2) ^ ROTR(a, 13)          */ \
    "and     6,23,%[a]\n\t"     /* r6  = (c ^ b) & a                       */ \
    "rotldi  8,%[a],25\n\t"     /* r8  = ROTR(a, 39)                       */ \
    "and     10,%[b],%[c]\n\t"  /* r10 = b & c                             */ \
    "xor     8,7,8\n\t"         /* r8  = S0(a)                             */ \
    "xor     10,6,10\n\t"       /* r10 = Maj(a,b,c)                        */ \
    "add     9,9,%[kpw]\n\t"    /* r9  = S1(e) + Ch(e, f, g) + K[j] + W[j] */ \
    "add     10,8,10\n\t"       /* r10 = T2 = S0(a) + Maj(a,b,c)           */ \
    "add     9,9,%[h]\n\t"      /* r9  = T1                                */ \
    "add     %[d],%[d],9\n\t"   /* d'  = d + T1                            */ \
    "add     %[h],10,9\n\t"     /* h'  = T2 + T1                           */ \
    : /* output list */                                                       \
      [d] "+r" ((_d)),                                                        \
      [h] "+r" ((_h))                                                         \
    : /* input list */                                                        \
      [a] "r" ((_a)),                                                         \
      [b] "r" ((_b)),                                                         \
      [c] "r" ((_c)),                                                         \
      [e] "r" ((_e)),                                                         \
      [f] "r" ((_f)),                                                         \
      [g] "r" ((_g)),                                                         \
      [kpw] "r" ((_kplusw))                                                   \
    : /* clobber list */                                                      \
      "r6", "r7", "r8", "r9", "r10", "r23"                                    \
  ); } while (0)

#define LOAD_W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _vRb, _j, _Rb, _w) do {\
  __asm__ volatile (                                                          \
    "lvsl    %[vrb],0,%[rb]\n\t"   /* parameter for vperm                  */ \
    "sldi    27,%[index],3\n\t"    /* j * 8 (double word size)             */ \
    "add     27,27,%[wptr]\n\t"    /* alias to W[j] location               */ \
    "addi    26,27,-128\n\t"                                                  \
    "lvx     %[w0],0,26\n\t"       /* load w[j-16] and w[j-15] to vector   */ \
    "addi    26,27,-112\n\t"                                                  \
    "lvx     %[w1],0,26\n\t"       /* load w[j-14] and w[j-13] to vector   */ \
    "addi    26,27,-96\n\t"                                                   \
    "lvx     %[w2],0,26\n\t"       /* load w[j-12] and w[j-11] to vector   */ \
    "addi    26,27,-80\n\t"                                                   \
    "lvx     %[w3],0,26\n\t"       /* load w[j-10] and w[j-9] to vector    */ \
    "addi    26,27,-64\n\t"                                                   \
    "lvx     %[w4],0,26\n\t"       /* load w[j-8] and w[j-7] to vector     */ \
    "addi    26,27,-48\n\t"                                                   \
    "lvx     %[w5],0,26\n\t"       /* load w[j-6] and w[j-5] to vector     */ \
    "addi    26,27,-32\n\t"                                                   \
    "lvx     %[w6],0,26\n\t"       /* load w[j-4] and w[j-3] to vector     */ \
    "addi    26,27,-16\n\t"                                                   \
    "lvx     %[w7],0,26\n\t"       /* load w[j-2] and w[j-1] to vector     */ \
    : /* output list */                                                       \
      [vrb] "=v" ((_vRb)),                                                    \
      [w0] "=v" ((_w0)),                                                      \
      [w1] "=v" ((_w1)),                                                      \
      [w2] "=v" ((_w2)),                                                      \
      [w3] "=v" ((_w3)),                                                      \
      [w4] "=v" ((_w4)),                                                      \
      [w5] "=v" ((_w5)),                                                      \
      [w6] "=v" ((_w6)),                                                      \
      [w7] "=v" ((_w7))                                                       \
    : /* input list */                                                        \
      [index] "r" ((_j)),                                                     \
      [rb] "r" ((_Rb)),                                                       \
      [wptr] "r" ((_w))                                                       \
    : /* clobber list */                                                      \
      "memory", "r26", "r27"                                                  \
  ); } while (0)

#define CALC_2W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _kpw0, _kpw1,         \
                _j, _vRb, _k) do {                                            \
  __asm__ volatile (                                                          \
    "sldi       27,%[index],3\n\t"          /* j * 8 (doubleword size)     */ \
    "add        26,27,%[kptr]\n\t"          /* alias to k[j] location      */ \
    "lvx        11,0,26\n\t"                                                  \
    "vperm      9,%[w1],%[w0],%[vrb]\n\t"   /* v9 = w[j-15] , w[j-14]      */ \
    "vperm      10,%[w5],%[w4],%[vrb]\n\t"  /* v10 = w[j-7] , w[j-6]       */ \
    /* v9 = s0(w[j-15]) , s0(w[j-14])                                      */ \
    "vshasigmad 9,9,0,0\n\t"                                                  \
    /* v12 = s1(w[j-2]) , s1(w[j-1])                                       */ \
    "vshasigmad 12,%[w7],0,0xf\n\t"                                           \
    /* v9 = s0(w[j-15]) + w[j-7] , s0(w[j-14]) + w[j-6]                    */ \
    "vaddudm    9,9,10\n\t"                                                   \
    /* v8 = s1(w[j-2]) + w[j-16] , s1(w[j-1]) + w[j-15]                    */ \
    "vaddudm    8,12,%[w0]\n\t"                                               \
    /* v9 = s0(w[j-15]) + w[j-7] + s1(w[j-2]) + w[j-16],  // w[j]          */ \
    /*      s0(w[j-14]) + w[j-6] + s1(w[j-1]) + w[j-15]   // w[j+1]        */ \
    "vaddudm    9,9,8\n\t"                                                    \
    /* Updating w0 to w7 to hold the "new previous" 16 values from w.      */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                        \
    "vor        %[w1],%[w2],%[w2]\n\t"                                        \
    "vor        %[w2],%[w3],%[w3]\n\t"                                        \
    "vor        %[w3],%[w4],%[w4]\n\t"                                        \
    "vor        %[w4],%[w5],%[w5]\n\t"                                        \
    "vor        %[w5],%[w6],%[w6]\n\t"                                        \
    "vor        %[w6],%[w7],%[w7]\n\t"                                        \
    "vor        %[w7],9,9\n\t"                                                \
    /* store k + w to v9 (2 values at once)                                */ \
    "vaddudm    9,9,11\n\t"                                                   \
    /* Move first doubleword in v9 to kpw1                                 */ \
    "mfvsrd     %[kpw1], 41\n\t"                                              \
    /* Move higher double word to low.                                     */ \
    "vperm      9,9,9,%[vrb]\n\t"                                             \
    /* Move first doubleword in v9 to kpw0                                 */ \
    "mfvsrd     %[kpw0], 41\n\t"                                              \
  : /* output list */                                                         \
    [w0] "+v" ((_w0)),                                                        \
    [w1] "+v" ((_w1)),                                                        \
    [w2] "+v" ((_w2)),                                                        \
    [w3] "+v" ((_w3)),                                                        \
    [w4] "+v" ((_w4)),                                                        \
    [w5] "+v" ((_w5)),                                                        \
    [w6] "+v" ((_w6)),                                                        \
    [w7] "+v" ((_w7)),                                                        \
    [kpw0] "=r" ((_kpw0)),                                                    \
    [kpw1] "=r" ((_kpw1))                                                     \
  : /* input list */                                                          \
    [index] "r" ((_j)),                                                       \
    [kptr] "r" ((_k)),                                                        \
    [vrb] "v" ((_vRb))                                                        \
  : /* clobber list */                                                        \
    "r26", "r27", "v8", "v9", "v10", "v11", "v12", "memory"                   \
  ); } while (0)

#endif

void sha2_transform(base_type* _h, base_type* w) {
  base_type a, b, c, d, e, f, g, h;
  int Rb = 8;
  vector int vRb;

  a = _h[0];
  b = _h[1];
  c = _h[2];
  d = _h[3];
  e = _h[4];
  f = _h[5];
  g = _h[6];
  h = _h[7];

  // Loop unrolling, from 0 to 15
  SHA2_ROUND(a, b, c, d, e, f, g, h, k[0] + w[0]);
  SHA2_ROUND(h, a, b, c, d, e, f, g, k[1] + w[1]);
  SHA2_ROUND(g, h, a, b, c, d, e, f, k[2] + w[2]);
  SHA2_ROUND(f, g, h, a, b, c, d, e, k[3] + w[3]);
  SHA2_ROUND(e, f, g, h, a, b, c, d, k[4] + w[4]);
  SHA2_ROUND(d, e, f, g, h, a, b, c, k[5] + w[5]);
  SHA2_ROUND(c, d, e, f, g, h, a, b, k[6] + w[6]);
  SHA2_ROUND(b, c, d, e, f, g, h, a, k[7] + w[7]);
  SHA2_ROUND(a, b, c, d, e, f, g, h, k[8] + w[8]);
  SHA2_ROUND(h, a, b, c, d, e, f, g, k[9] + w[9]);
  SHA2_ROUND(g, h, a, b, c, d, e, f, k[10] + w[10]);
  SHA2_ROUND(f, g, h, a, b, c, d, e, k[11] + w[11]);
  SHA2_ROUND(e, f, g, h, a, b, c, d, k[12] + w[12]);
  SHA2_ROUND(d, e, f, g, h, a, b, c, k[13] + w[13]);
  SHA2_ROUND(c, d, e, f, g, h, a, b, k[14] + w[14]);
  SHA2_ROUND(b, c, d, e, f, g, h, a, k[15] + w[15]);

#if SHA_BITS == 256
  base_type kpw0, kpw1, kpw2, kpw3;
  vector_base_type w0, w1, w2, w3;

  int Rc = 4;
  vector int vRc;

  // Load 16 elements from w out of the loop
  int j = 16;
  LOAD_W(w0, w1, w2, w3, vRb, vRc, j, Rb, Rc, w);

  // From 16 to W_SIZE (64) in 8 steps
  while (j < W_SIZE) {
    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, vRb, vRc, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);
    j += 4;

    CALC_4W(w0, w1, w2, w3, kpw0, kpw1, kpw2, kpw3, j, vRb, vRc, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);
    j += 4;
  }

#else // SHA_BITS == 512

  vector_base_type w0, w1, w2, w3, w4, w5, w6, w7;
  base_type kpw0, kpw1;

  // Load 16 elements from w out of the loop
  int j = 16;
  LOAD_W(w0, w1, w2, w3, w4, w5, w6, w7, vRb, j, Rb, w);

  // From 16 to W_SIZE (64) in 8 steps
  while (j < W_SIZE) {
    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw0);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, vRb, k);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw0);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);
    j += 2;
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
