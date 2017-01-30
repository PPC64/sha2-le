#ifndef _PPC64_LE_SHA2_LL_ASM_H_
#define _PPC64_LE_SHA2_LL_ASM_H_

#if LOW_LEVEL != 2
#error "The sha2_ll_asm.h header should only be included on LOW_LEVEL == 2"
#endif

#include "base-types.h"

/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */

#if SHA_BITS == 256

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {                   \
  base_type t0;                                                                    \
  base_type t1;                                                                    \
  base_type t2;                                                                    \
  base_type t3;                                                                    \
  base_type t4;                                                                    \
  base_type t5;                                                                    \
  __asm__ volatile (                                                               \
    "rotlwi  %[t4],%[e],%[c1]\n\t"   /* t4  = ROTR(e, 6)                        */ \
    "rotlwi  %[t2],%[e],%[c2]\n\t"   /* t2  = ROTR(e, 11)                       */ \
    "and     %[t1],%[f],%[e]\n\t"    /* t1  = e & f                             */ \
    "xor     %[t2],%[t4],%[t2]\n\t"  /* t2  = ROTR(e, 6) ^ ROTR(e, 11)          */ \
    "andc    %[t3],%[g],%[e]\n\t"    /* t3  = !e & g                            */ \
    "rotlwi  %[t4],%[e],%[c3]\n\t"   /* t4  = ROTR(e, 25)                       */ \
    "rotlwi  %[t0],%[a],%[c4]\n\t"   /* t0  = ROTR(a, 2)                        */ \
    "xor     %[t4],%[t2],%[t4]\n\t"  /* t4  = S1(e)                             */ \
    "xor     %[t3],%[t3],%[t1]\n\t"  /* t3  = Ch(e, f, g)                       */ \
    "xor     %[t5],%[c],%[b]\n\t"    /* t5  = c ^ b                             */ \
    "rotlwi  %[t2],%[a],%[c5]\n\t"   /* t2  = ROTR(a, 13)                       */ \
    "add     %[t3],%[t4],%[t3]\n\t"  /* t3  = S1(e) + Ch(e, f, g)               */ \
    "xor     %[t1],%[t0],%[t2]\n\t"  /* t1  = ROTR(a, 2) ^ ROTR(a, 13)          */ \
    "and     %[t0],%[t5],%[a]\n\t"   /* t0  = (c ^ b) & a                       */ \
    "rotlwi  %[t2],%[a],%[c6]\n\t"   /* t2  = ROTR(a, 22)                       */ \
    "and     %[t4],%[b],%[c]\n\t"    /* t4  = b & c                             */ \
    "xor     %[t2],%[t1],%[t2]\n\t"  /* t2  = S0(a)                             */ \
    "xor     %[t4],%[t0],%[t4]\n\t"  /* t4  = Maj(a,b,c)                        */ \
    "add     %[t3],%[t3],%[kpw]\n\t" /* t3  = S1(e) + Ch(e, f, g) + K[j] + W[j] */ \
    "add     %[t4],%[t2],%[t4]\n\t"  /* t4  = T2 = S0(a) + Maj(a,b,c)           */ \
    "add     %[t3],%[t3],%[h]\n\t"   /* t3  = T1                                */ \
    "add     %[t2],%[d],%[t3]\n\t"   /* t2 = d + T1                             */ \
    "clrldi  %[d],%[t2],%[c7]\n\t"   /* d'  = (base_type)(d + T1)               */ \
    "add     %[t3],%[t4],%[t3]\n\t"  /* t3  = T2 + T1                           */ \
    "clrldi  %[h],%[t3],%[c7]\n\t"   /* h'  = (base_type)(T2 + T1)              */ \
    : /* output list */                                                            \
      [d] "+r" ((_d)),                                                             \
      [h] "+r" ((_h)),                                                             \
      [t0] "=&r" (t0),                                                             \
      [t1] "=&r" (t1),                                                             \
      [t2] "=&r" (t2),                                                             \
      [t3] "=&r" (t3),                                                             \
      [t4] "=&r" (t4),                                                             \
      [t5] "=&r" (t5)                                                              \
    : /* input list */                                                             \
      [a] "r" ((_a)),                                                              \
      [b] "r" ((_b)),                                                              \
      [c] "r" ((_c)),                                                              \
      [e] "r" ((_e)),                                                              \
      [f] "r" ((_f)),                                                              \
      [g] "r" ((_g)),                                                              \
      [kpw] "r" ((_kplusw)),                                                       \
      [c1] "i" (32 - 6),                                                           \
      [c2] "i" (32 - 11),                                                          \
      [c3] "i" (32 - 25),                                                          \
      [c4] "i" (32 - 2),                                                           \
      [c5] "i" (32 - 13),                                                          \
      [c6] "i" (32 - 22),                                                          \
      [c7] "i" (32)                                                                \
    : /* clobber list */                                                           \
                                                                                   \
  ); } while (0)

#define DEQUE(_k, _kpw0, _kpw1, _kpw2, _kpw3,  _vrb) do {                     \
  vector_base_type vt0;                                                       \
  __asm__ volatile (                                                          \
    /* Move first doubleword in v0 to kpw1                                 */ \
    "mfvrd      %[kpw2], %[k]\n\t"                                            \
    /* Move low word to kpw3                                               */ \
    "srdi       %[kpw3], %[kpw2], %[c1]\n\t"                                  \
    /* Clear low word. Keep high word.                                     */ \
    "clrldi     %[kpw2], %[kpw2], %[c1]\n\t"                                  \
    /* Move higher double word to low.                                     */ \
    "vperm      %[vt0], %[k], %[k], %[vrb]\n\t"                               \
    /* Move first doubleword in v0 to kpw0                                 */ \
    "mfvrd      %[kpw0], %[vt0]\n\t"                                          \
    /* Move low word to kpw1                                               */ \
    "srdi       %[kpw1], %[kpw0], %[c1]\n\t"                                  \
    /* Clear low word. Keep high word.                                     */ \
    "clrldi     %[kpw0], %[kpw0], %[c1]\n\t"                                  \
    : /* output list */                                                       \
      [kpw0] "=r" ((_kpw0)),                                                  \
      [kpw1] "=r" ((_kpw1)),                                                  \
      [kpw2] "=r" ((_kpw2)),                                                  \
      [kpw3] "=r" ((_kpw3)),                                                  \
      [vt0]  "=&v" (vt0)                                                      \
    : /* input list */                                                        \
      [k] "v" ((_k)),                                                         \
      [vrb] "v" ((_vrb)),                                                     \
      [c1] "i" (32)                                                           \
    : /* clobber list */                                                      \
      "memory"                                                                \
  ); } while (0)

#define LOAD_W_PLUS_K(_k0, _k1, _k2, _k3, _w0, _w1, _w2, _w3, _vRb, _vRc, _j,   \
    _Rb, _Rc, _w, _k) do {                                                      \
  base_type t0;                                                                 \
  base_type t1;                                                                 \
  vector_base_type vt0;                                                         \
  vector_base_type vt1;                                                         \
  vector_base_type vt2;                                                         \
  vector_base_type vt3;                                                         \
  __asm__ volatile (                                                            \
    "sldi    %[t1],%[index],%[c1]\n\t"  /* j * 4 (word size)                 */ \
    "add     %[t1],%[t1],%[wptr]\n\t"   /* alias to W[j] location            */ \
    "addi    %[t0],%[t1],%[c2]\n\t"                                             \
    "lvx     %[w0],0,%[t0]\n\t"         /* load w[j-16] to w[j-13] to vector */ \
    "addi    %[t0],%[t1],%[c3]\n\t"                                             \
    "lvx     %[w1],0,%[t0]\n\t"         /* load w[j-12] to w[j-9] to vector  */ \
    "addi    %[t0],%[t1],%[c4]\n\t"                                             \
    "lvx     %[w2],0,%[t0]\n\t"         /* load w[j-8] to w[j-5] to vector   */ \
    "addi    %[t0],%[t1],%[c5]\n\t"                                             \
    "lvx     %[w3],0,%[t0]\n\t"         /* load w[j-4] to w[j-1] to vector   */ \
    /* Load 4*4 k values                                                     */ \
    "sldi    %[t1],%[index],%[c1]\n\t"  /* j * 4 (word size)                 */ \
    "add     %[t1],%[t1],%[kptr]\n\t"   /* alias to k[j] location            */ \
    "addi    %[t0],%[t1],%[c2]\n\t"                                             \
    "lvx     %[vt0],0,%[t0]\n\t"        /* load k[j-16] to k[j-13] to vector */ \
    "addi    %[t0],%[t1],%[c3]\n\t"                                             \
    "lvx     %[vt1],0,%[t0]\n\t"        /* load k[j-12] to k[j-9] to vector  */ \
    "addi    %[t0],%[t1],%[c4]\n\t"                                             \
    "lvx     %[vt2],0,%[t0]\n\t"        /* load k[j-8] to k[j-5] to vector   */ \
    "addi    %[t0],%[t1],%[c5]\n\t"                                             \
    "lvx     %[vt3],0,%[t0]\n\t"        /* load k[j-4] to k[j-1] to vector   */ \
                                                                                \
    "lvsl    %[vrb],0,%[rb]\n\t"        /* parameter for vperm               */ \
    "lvsr    %[vrc],0,%[rc]\n\t"        /* parameter for vperm               */ \
    /* Add _w to k                                                           */ \
    "vadduwm    %[k0],%[vt0],%[w0]\n\t"                                         \
    "vadduwm    %[k1],%[vt1],%[w1]\n\t"                                         \
    "vadduwm    %[k2],%[vt2],%[w2]\n\t"                                         \
    "vadduwm    %[k3],%[vt3],%[w3]\n\t"                                         \
    : /* output list */                                                         \
      [k0] "=v" ((_k0)),                                                        \
      [k1] "=v" ((_k1)),                                                        \
      [k2] "=v" ((_k2)),                                                        \
      [k3] "=v" ((_k3)),                                                        \
      [w0] "=v" ((_w0)),                                                        \
      [w1] "=v" ((_w1)),                                                        \
      [w2] "=v" ((_w2)),                                                        \
      [w3] "=v" ((_w3)),                                                        \
      [vrb] "=v" ((_vRb)),                                                      \
      [vrc] "=v" ((_vRc)),                                                      \
      [t0] "=&r" (t0),                                                          \
      [t1] "=&r" (t1),                                                          \
      [vt0] "=&v" (vt0),                                                        \
      [vt1] "=&v" (vt1),                                                        \
      [vt2] "=&v" (vt2),                                                        \
      [vt3] "=&v" (vt3)                                                         \
    : /* input list */                                                          \
      [index] "r" ((_j)),                                                       \
      [rb] "r" ((_Rb)),                                                         \
      [rc] "r" ((_Rc)),                                                         \
      [wptr] "r" ((_w)),                                                        \
      [kptr] "r" ((_k)),                                                        \
      [c1] "i" (2),                                                             \
      [c2] "i" (-64),                                                           \
      [c3] "i" (-48),                                                           \
      [c4] "i" (-32),                                                           \
      [c5] "i" (-16)                                                            \
    : /* clobber list */                                                        \
      "memory"                                                                  \
  ); } while (0)

#define CALC_4W(_w0, _w1, _w2, _w3, _kpw0, _kpw1, _kpw2, _kpw3,               \
                _j, _vRb, _vRc, _k) do {                                      \
  base_type t0;                                                               \
  base_type t1;                                                               \
  vector_base_type vt0;                                                       \
  vector_base_type vt1;                                                       \
  vector_base_type vt2;                                                       \
  vector_base_type vt3;                                                       \
  vector_base_type vt4;                                                       \
  vector_base_type vt5;                                                       \
  vector_base_type vt6;                                                       \
  vector_base_type vt7;                                                       \
  vector_base_type vt8;                                                       \
  __asm__ volatile (                                                          \
    "sldi       %[t1],%[index],2\n\t"           /* j * 4 (word size)       */ \
    "add        %[t0],%[t1],%[kptr]\n\t"        /* alias to k[j] location  */ \
    "lvx        %[vt6],0,%[t0]\n\t"                                           \
    /* vt1 = w[j-15], w[j-14], w[j-13], w[j-12]                            */ \
    "vperm      %[vt1],%[w1],%[w0],%[vrc]\n\t"                                \
    /* vt7 = w[j-7], w[j-6], w[j-5], w[j-4]                                */ \
    "vperm      %[vt7],%[w3],%[w2],%[vrc]\n\t"                                \
    /* vt8 = w[j-2], w[j-1], w[j-4], w[j-3]                                */ \
    "vperm      %[vt8],%[w3],%[w3],%[vrb]\n\t"                                \
    /* vt1 = s0(w[j-15]) , s0(w[j-14]) , s0(w[j-13]) , s0(w[j-12])         */ \
    "vshasigmaw %[vt1],%[vt1],0,0\n\t"                                        \
    /* vt2 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j-4]) , s1(w[j-3])             */ \
    "vshasigmaw %[vt2],%[vt8],0,%[six1]\n\t"                                  \
    /* vt3 = s0(w[j-15]) + w[j-7],                                         */ \
    /*       s0(w[j-14]) + w[j-6],                                         */ \
    /*       s0(w[j-13]) + w[j-5],                                         */ \
    /*       s0(w[j-12]) + w[j-4]                                          */ \
    "vadduwm    %[vt3],%[vt1],%[vt7]\n\t"                                     \
    /* vt4 = s0(w[j-15]) + w[j-7] + w[j-16],                               */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15],                               */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14],                               */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13]                                */ \
    "vadduwm    %[vt4],%[vt3],%[w0]\n\t"                                      \
    /* vt5 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]          */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]        */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j-4]), // UNDEFINED     */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j-3])  // UNDEFINED     */ \
    "vadduwm    %[vt5],%[vt4],%[vt2]\n\t"                                     \
    /* At this point, v9[0] and v9[1] are the correct values to be         */ \
    /* stored at w[j] and w[j+1].                                          */ \
    /* vt5[2] and vt5[3] are not considered                                */ \
    /* vt0 = s1(w[j]) , s1(s(w[j+1]) , UNDEFINED , UNDEFINED               */ \
    "vshasigmaw %[vt0],%[vt5],0,%[six1]\n\t"                                  \
    /* vt2 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j]) , s1(w[j+1])               */ \
    /*  NOTE: %x[vtN] corresponds to the equivalent VSX register           */ \
    "xxpermdi   %x[vt2],%x[vt0],%x[vt2],%[c1]\n\t"                            \
    /* vt5 = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]          */ \
    /*       s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]        */ \
    /*       s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j]),   // w[j+2]        */ \
    /*       s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j+1])  // w[j+4]        */ \
    "vadduwm    %[vt5],%[vt4],%[vt2]\n\t"                                     \
    /* Updating w0 to w3 to hold the "new previous" 16 values from w.      */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                        \
    "vor        %[w1],%[w2],%[w2]\n\t"                                        \
    "vor        %[w2],%[w3],%[w3]\n\t"                                        \
    "vor        %[w3],%[vt5],%[vt5]\n\t"                                      \
    /* store k + w to vt5 (4 values at once)                               */ \
    "vadduwm    %[vt5],%[vt5],%[vt6]\n\t"                                     \
    /* Move low doubleword in vt5 to kpw2                                  */ \
    "mfvsrd     %[kpw2], %x[vt5]\n\t"                                         \
    /* Move low word to kpw3                                               */ \
    "srdi       %[kpw3], %[kpw2], %[c2]\n\t"                                  \
    /* Clear low word. Keep high word.                                     */ \
    "clrldi     %[kpw2], %[kpw2], %[c2]\n\t"                                  \
    /* Move high doubleword to low.                                        */ \
    "vperm      %[vt5],%[vt5],%[vt5],%[vrb]\n\t"                              \
    /* Move high doubleword in vt5 to kpw0                                 */ \
    "mfvsrd     %[kpw0], %x[vt5]\n\t"                                         \
    /* Move higher word to kpw2                                            */ \
    "srdi       %[kpw1], %[kpw0], %[c2]\n\t"                                  \
    /* Clear higher word. Keep highest word.                               */ \
    "clrldi     %[kpw0], %[kpw0], %[c2]\n\t"                                  \
    : /* output list */                                                       \
      [w0] "+v" ((_w0)),                                                      \
      [w1] "+v" ((_w1)),                                                      \
      [w2] "+v" ((_w2)),                                                      \
      [w3] "+v" ((_w3)),                                                      \
      [kpw0] "=r" ((_kpw0)),                                                  \
      [kpw1] "=r" ((_kpw1)),                                                  \
      [kpw2] "=r" ((_kpw2)),                                                  \
      [kpw3] "=r" ((_kpw3)),                                                  \
      [t0] "=&r" (t0),                                                        \
      [t1] "=&r" (t1),                                                        \
      [vt0] "=&v" (vt0),                                                      \
      [vt1] "=&v" (vt1),                                                      \
      [vt2] "=&v" (vt2),                                                      \
      [vt3] "=&v" (vt3),                                                      \
      [vt4] "=&v" (vt4),                                                      \
      [vt5] "=&v" (vt5),                                                      \
      [vt6] "=&v" (vt6),                                                      \
      [vt7] "=&v" (vt7),                                                      \
      [vt8] "=&v" (vt8)                                                       \
    : /* input list */                                                        \
      [index] "r" ((_j)),                                                     \
      [vrb] "v" ((_vRb)),                                                     \
      [vrc] "v" ((_vRc)),                                                     \
      [kptr] "r" ((_k)),                                                      \
      [c1] "i" (3),                                                           \
      [c2] "i" (32),                                                          \
      [six1] "i" (0xf)                                                        \
    : /* clobber list */                                                      \
      "memory"                                                                \
  ); } while (0)

#else // SHA_BITS == 512

#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {                   \
  base_type t0;                                                                    \
  base_type t1;                                                                    \
  base_type t2;                                                                    \
  base_type t3;                                                                    \
  base_type t4;                                                                    \
  base_type t5;                                                                    \
  __asm__ volatile (                                                               \
    "rotldi  %[t4],%[e],%[c1]\n\t"   /* t4 = ROTR(e, 14)                        */ \
    "rotldi  %[t2],%[e],%[c2]\n\t"   /* t2 = ROTR(e, 18)                        */ \
    "and     %[t1],%[f],%[e]\n\t"    /* t1 = e & f                              */ \
    "xor     %[t2],%[t4],%[t2]\n\t"  /* t2 = ROTR(e, 14) ^ ROTR(e, 18)          */ \
    "andc    %[t3],%[g],%[e]\n\t"    /* t3 = !e & g                             */ \
    "rotldi  %[t4],%[e],%[c3]\n\t"   /* t4 = ROTR(e, 41)                        */ \
    "rotldi  %[t0],%[a],%[c4]\n\t"   /* t0 = ROTR(a, 28)                        */ \
    "xor     %[t4],%[t2],%[t4]\n\t"  /* t4 = S1(e)                              */ \
    "xor     %[t3],%[t3],%[t1]\n\t"  /* t3 = Ch(e, f, g)                        */ \
    "xor     %[t5],%[c],%[b]\n\t"    /* t5 = c ^ b                              */ \
    "rotldi  %[t2],%[a],%[c5]\n\t"   /* t2 = ROTR(a, 34)                        */ \
    "add     %[t3],%[t4],%[t3]\n\t"  /* t3 = S1(e) + Ch(e, f, g)                */ \
    "xor     %[t1],%[t0],%[t2]\n\t"  /* t2 = ROTR(a, 2) ^ ROTR(a, 13)           */ \
    "and     %[t0],%[t5],%[a]\n\t"   /* t0 = (c ^ b) & a                        */ \
    "rotldi  %[t2],%[a],%[c6]\n\t"   /* t2 = ROTR(a, 39)                        */ \
    "and     %[t4],%[b],%[c]\n\t"    /* t4 = b & c                              */ \
    "xor     %[t2],%[t1],%[t2]\n\t"  /* t2 = S0(a)                              */ \
    "xor     %[t4],%[t0],%[t4]\n\t"  /* t4 = Maj(a,b,c)                         */ \
    "add     %[t3],%[t3],%[kpw]\n\t" /* t3 = S1(e) + Ch(e, f, g) + K[j] + W[j]  */ \
    "add     %[t4],%[t2],%[t4]\n\t"  /* t4 = T2 = S0(a) + Maj(a,b,c)            */ \
    "add     %[t3],%[t3],%[h]\n\t"   /* t3 = T1                                 */ \
    "add     %[d],%[d],%[t3]\n\t"    /* d' = d + T1                             */ \
    "add     %[h],%[t4],%[t3]\n\t"   /* h' = T2 + T1                            */ \
    : /* output list */                                                            \
      [d] "+r" ((_d)),                                                             \
      [h] "+r" ((_h)),                                                             \
      [t0] "=&r" (t0),                                                             \
      [t1] "=&r" (t1),                                                             \
      [t2] "=&r" (t2),                                                             \
      [t3] "=&r" (t3),                                                             \
      [t4] "=&r" (t4),                                                             \
      [t5] "=&r" (t5)                                                              \
    : /* input list */                                                             \
      [a] "r" ((_a)),                                                              \
      [b] "r" ((_b)),                                                              \
      [c] "r" ((_c)),                                                              \
      [e] "r" ((_e)),                                                              \
      [f] "r" ((_f)),                                                              \
      [g] "r" ((_g)),                                                              \
      [kpw] "r" ((_kplusw)),                                                       \
      [c1] "i" (64 -14),                                                           \
      [c2] "i" (64 -18),                                                           \
      [c3] "i" (64 -41),                                                           \
      [c4] "i" (64 -28),                                                           \
      [c5] "i" (64 -34),                                                           \
      [c6] "i" (64 -39)                                                            \
    : /* clobber list */                                                           \
                                                                                   \
  ); } while (0)

#define LOAD_W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _vRb, _j, _Rb, _w) do  { \
  base_type t0;                                                                 \
  base_type t1;                                                                 \
  __asm__ volatile (                                                            \
    "lvsl    %[vrb],0,%[rb]\n\t"       /* parameter for vperm                */ \
    "sldi    %[t1],%[index],%[c1]\n\t" /* j * 8 (double word size)           */ \
    "add     %[t1],%[t1],%[wptr]\n\t"  /* alias to W[j] location             */ \
    "addi    %[t0],%[t1],%[c2]\n\t"                                             \
    "lvx     %[w0],0,%[t0]\n\t"        /* load w[j-16] and w[j-15] to vector */ \
    "addi    %[t0],%[t1],%[c3]\n\t"                                             \
    "lvx     %[w1],0,%[t0]\n\t"        /* load w[j-14] and w[j-13] to vector */ \
    "addi    %[t0],%[t1],%[c4]\n\t"                                             \
    "lvx     %[w2],0,%[t0]\n\t"        /* load w[j-12] and w[j-11] to vector */ \
    "addi    %[t0],%[t1],%[c5]\n\t"                                             \
    "lvx     %[w3],0,%[t0]\n\t"        /* load w[j-10] and w[j-9] to vector  */ \
    "addi    %[t0],%[t1],%[c6]\n\t"                                             \
    "lvx     %[w4],0,%[t0]\n\t"        /* load w[j-8] and w[j-7] to vector   */ \
    "addi    %[t0],%[t1],%[c7]\n\t"                                             \
    "lvx     %[w5],0,%[t0]\n\t"        /* load w[j-6] and w[j-5] to vector   */ \
    "addi    %[t0],%[t1],%[c8]\n\t"                                             \
    "lvx     %[w6],0,%[t0]\n\t"        /* load w[j-4] and w[j-3] to vector   */ \
    "addi    %[t0],%[t1],%[c9]\n\t"                                             \
    "lvx     %[w7],0,%[t0]\n\t"        /* load w[j-2] and w[j-1] to vector   */ \
    : /* output list */                                                         \
      [vrb] "=v" ((_vRb)),                                                      \
      [w0] "=v" ((_w0)),                                                        \
      [w1] "=v" ((_w1)),                                                        \
      [w2] "=v" ((_w2)),                                                        \
      [w3] "=v" ((_w3)),                                                        \
      [w4] "=v" ((_w4)),                                                        \
      [w5] "=v" ((_w5)),                                                        \
      [w6] "=v" ((_w6)),                                                        \
      [w7] "=v" ((_w7)),                                                        \
      [t0] "=&r" (t0),                                                          \
      [t1] "=&r" (t1)                                                           \
    : /* input list */                                                          \
      [index] "r" ((_j)),                                                       \
      [rb] "r" ((_Rb)),                                                         \
      [wptr] "r" ((_w)),                                                        \
      [c1] "i" (3),                                                             \
      [c2] "i" (-128),                                                          \
      [c3] "i" (-112),                                                          \
      [c4] "i" (-96),                                                           \
      [c5] "i" (-80),                                                           \
      [c6] "i" (-64),                                                           \
      [c7] "i" (-48),                                                           \
      [c8] "i" (-32),                                                           \
      [c9] "i" (-16)                                                            \
    : /* clobber list */                                                        \
      "memory"                                                                  \
  ); } while (0)

#define CALC_2W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _kpw0, _kpw1,         \
                _j, _vRb, _k) do {                                            \
  base_type t0;                                                               \
  base_type t1;                                                               \
  vector_base_type vt0;                                                       \
  vector_base_type vt1;                                                       \
  vector_base_type vt2;                                                       \
  vector_base_type vt3;                                                       \
  vector_base_type vt4;                                                       \
  __asm__ volatile (                                                          \
    "sldi       %[t1],%[index],%[c1]\n\t"      /* j * 8 (doubleword size)  */ \
    "add        %[t0],%[t1],%[kptr]\n\t"       /* alias to k[j] location   */ \
    "lvx        %[vt3],0,%[t0]\n\t"                                           \
    "vperm      %[vt1],%[w1],%[w0],%[vrb]\n\t" /* vt1 = w[j-15] , w[j-14]  */ \
    "vperm      %[vt2],%[w5],%[w4],%[vrb]\n\t" /* vt2 = w[j-7] , w[j-6]    */ \
    /* vt1 = s0(w[j-15]) , s0(w[j-14])                                     */ \
    "vshasigmad %[vt1],%[vt1],0,0\n\t"                                        \
    /* vt4 = s1(w[j-2]) , s1(w[j-1])                                       */ \
    "vshasigmad %[vt4],%[w7],0,%[six1]\n\t"                                   \
    /* vt1 = s0(w[j-15]) + w[j-7] , s0(w[j-14]) + w[j-6]                   */ \
    "vaddudm    %[vt1],%[vt1],%[vt2]\n\t"                                     \
    /* vt0 = s1(w[j-2]) + w[j-16] , s1(w[j-1]) + w[j-15]                   */ \
    "vaddudm    %[vt0],%[vt4],%[w0]\n\t"                                      \
    /* vt1 = s0(w[j-15]) + w[j-7] + s1(w[j-2]) + w[j-16],  // w[j]         */ \
    /*       s0(w[j-14]) + w[j-6] + s1(w[j-1]) + w[j-15]   // w[j+1]       */ \
    "vaddudm    %[vt1],%[vt1],%[vt0]\n\t"                                     \
    /* Updating w0 to w7 to hold the "new previous" 16 values from w.      */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                        \
    "vor        %[w1],%[w2],%[w2]\n\t"                                        \
    "vor        %[w2],%[w3],%[w3]\n\t"                                        \
    "vor        %[w3],%[w4],%[w4]\n\t"                                        \
    "vor        %[w4],%[w5],%[w5]\n\t"                                        \
    "vor        %[w5],%[w6],%[w6]\n\t"                                        \
    "vor        %[w6],%[w7],%[w7]\n\t"                                        \
    "vor        %[w7],%[vt1],%[vt1]\n\t"                                      \
    /* store k + w to vt1 (2 values at once)                               */ \
    "vaddudm    %[vt1],%[vt1],%[vt3]\n\t"                                     \
    /* Move first doubleword in vt1 to kpw1                                */ \
    "mfvsrd     %[kpw1], %x[vt1]\n\t"                                         \
    /* Move higher double word to low.                                     */ \
    "vperm      %[vt1],%[vt1],%[vt1],%[vrb]\n\t"                              \
    /* Move first doubleword in vt1 to kpw0                                */ \
    "mfvsrd     %[kpw0], %x[vt1]\n\t"                                         \
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
    [kpw1] "=r" ((_kpw1)),                                                    \
    [t0] "=&r" (t0),                                                          \
    [t1] "=&r" (t1),                                                          \
    [vt0] "=&v" (vt0),                                                        \
    [vt1] "=&v" (vt1),                                                        \
    [vt2] "=&v" (vt2),                                                        \
    [vt3] "=&v" (vt3),                                                        \
    [vt4] "=&v" (vt4)                                                         \
  : /* input list */                                                          \
    [index] "r" ((_j)),                                                       \
    [kptr] "r" ((_k)),                                                        \
    [vrb] "v" ((_vRb)),                                                       \
    [c1] "i" (3),                                                             \
    [six1] "i" (0xf)                                                          \
  : /* clobber list */                                                        \
    "memory"                                                                  \
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


#if SHA_BITS == 256

  base_type kpw0, kpw1, kpw2, kpw3;

  vector_base_type w0, w1, w2, w3;
  vector_base_type kplusw0, kplusw1, kplusw2, kplusw3;

  // Load 16 elements from w out of the loop
  int Rc = 4;
  vector int vRc;
  int j = 16;
  LOAD_W_PLUS_K(kplusw0, kplusw1, kplusw2, kplusw3, w0, w1, w2, w3, vRb, vRc, j, Rb, Rc, w, k);

  // Loop unrolling, from 0 to 15
  DEQUE  (kplusw0, kpw0, kpw1, kpw2, kpw3, vRb);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw1, kpw0, kpw1, kpw2, kpw3, vRb);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

  DEQUE  (kplusw2, kpw0, kpw1, kpw2, kpw3, vRb);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kpw2);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw3);

  DEQUE  (kplusw3, kpw0, kpw1, kpw2, kpw3, vRb);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kpw2);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw3);

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

  // Load 16 elements from w out of the loop
  int j = 16;
  LOAD_W(w0, w1, w2, w3, w4, w5, w6, w7, vRb, j, Rb, w);

  // From 16 to W_SIZE (80) in 8 steps
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

#endif // _PPC64_LE_SHA2_LL_ASM_H_
