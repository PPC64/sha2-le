#include "sha2_compress.h"

/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous ones.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */
#define SHA2_ROUND(_a, _b, _c, _d, _e, _f, _g, _h, _kplusw) do {            \
    vector_base_type ch;                                                    \
    vector_base_type maj;                                                   \
    vector_base_type bsa;                                                   \
    vector_base_type bse;                                                   \
    vector_base_type vt1;                                                   \
    vector_base_type vt2;                                                   \
    vector_base_type vt3;                                                   \
    vector_base_type vt4;                                                   \
  __asm__ volatile (                                                        \
      "vsel %[ch],%[g],%[f],%[e]\n\t"       /* ch   = Ch(e,f,g)          */ \
      "vxor %[maj],%[a],%[b]\n\t"           /* intermediate Maj          */ \
      "vshasigmad %[bse],%[e],1,0xf\n\t"    /* bse  = BigSigma1(e)       */ \
      "vaddudm %[vt2],%[ch],%[kpw]\n\t"     /* vt2  = ch + kpw           */ \
      "vaddudm %[vt1],%[h],%[bse]\n\t"      /* vt1  = h + bse            */ \
      "vsel %[maj],%[b],%[c],%[maj]\n\t"    /* maj  = Maj(a,b,c)         */ \
      "vaddudm %[vt3],%[vt1],%[vt2]\n\t"    /* vt3  = h + bse + ch + kpw */ \
      "vshasigmad %[bsa],%[a],1,0\n\t"      /* bsa  = BigSigma0(a)       */ \
      "vaddudm %[vt4],%[bsa],%[maj]\n\t"    /* vt4  = bsa + maj          */ \
      "vaddudm %[d],%[d],%[vt3]\n\t"        /* d    = d + vt3            */ \
      "vaddudm %[h],%[vt3],%[vt4]\n\t"      /* h    = vt3 + vt4          */ \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [ch] "=&v" (ch),                                                      \
      [maj] "=&v" (maj),                                                    \
      [bsa] "=&v" (bsa),                                                    \
      [bse] "=&v" (bse),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      /* outputs/inputs                                                  */ \
      [d] "+v" ((_d)),                                                      \
      [h] "+v" ((_h))                                                       \
    : /* input list                                                      */ \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [kpw] "v" ((_kplusw))                                                 \
    : /* clobber list                                                    */ \
                                                                            \
  ); } while (0)

#define LOAD_H_VEC(_a, _b, _c, _d, _e, _f, _g, _h, _hptr, _vrb) do {        \
  vector_base_type tmp1;                                                    \
  base_type index;                                                          \
  __asm__ volatile (                                                        \
     "lvx %[a],0,%[hptr]\n\t"          /* load unaligned                 */ \
     "addi %[idx],%[hptr],16\n\t"                                           \
     "lvsr %[vrb],0,%[idx]\n\t"        /* set vrb according to alignment */ \
     "lvx %[c],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[a],%[c],%[a],%[vrb]\n\t" /* a = {a,b}                      */ \
     "addi %[idx],%[hptr],32\n\t"                                           \
     "lvx %[e],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[c],%[e],%[c],%[vrb]\n\t" /* c = {c,d}                      */ \
     "addi %[idx],%[hptr],48\n\t"                                           \
     "lvx %[g],0,%[idx]\n\t"           /* load unaligned                 */ \
     "vperm %[e],%[g],%[e],%[vrb]\n\t" /* e = {e,f}                      */ \
     "addi %[idx],%[hptr],64\n\t"                                           \
     "lvx %[tmp1],0,%[idx]\n\t"        /* load unaligned                 */ \
     /* g = {g,h}                                                        */ \
     "vperm %[g],%[tmp1],%[g],%[vrb]\n\t"                                   \
     "vsldoi %[b],%[a],%[a],8 \n\t"    /* b = {b,a}                      */ \
     "vsldoi %[d],%[c],%[c],8 \n\t"    /* d = {d,c}                      */ \
     "vsldoi %[f],%[e],%[e],8 \n\t"    /* f = {f,e}                      */ \
     "vsldoi %[h],%[g],%[g],8 \n\t"    /* h = {h,g}                      */ \
   : /* output list                                                      */ \
     /* temporaries                                                      */ \
     [idx] "=&b" (index),                                                   \
     [tmp1] "=&v" ((tmp1)),                                                 \
     /* actual outputs                                                   */ \
     /* a,c,e,g are read in the asm, hence should be reserved            */ \
     [a] "=&v" ((_a)),                                                      \
     [c] "=&v" ((_c)),                                                      \
     [e] "=&v" ((_e)),                                                      \
     [g] "=&v" ((_g)),                                                      \
     [vrb] "=&v" ((_vrb)),                                                  \
     [b] "=v" ((_b)),                                                       \
     [d] "=v" ((_d)),                                                       \
     [f] "=v" ((_f)),                                                       \
     [h] "=v" ((_h))                                                        \
   : /* input list                                                       */ \
     [hptr] "b" ((_hptr))                                                   \
   : /* clobber list                                                     */ \
     "memory"                                                               \
  ); } while (0)                                                            \

#define DEQUE(_k, _kpw1) do {                                        \
  __asm__ volatile (                                                        \
    "vsldoi      %[kpw1], %[k], %[k], 8\n\t" /* kpw1 = {k[1],k[0]}       */ \
    : /* output list                                                     */ \
      [kpw1] "=v" ((_kpw1)),                                                \
      /* k is read in the asm, hence should be reserved                  */ \
      [k] "+&v" ((_k))                                                      \
    : /* input list                                                      */ \
    : /* clobber list                                                    */ \
  ); } while (0)

#define LOAD_W_PLUS_K(_k0, _k1, _k2, _k3, _k4, _k5, _k6, _k7,               \
                      _w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7,               \
                      _vRb, _j, _k, _w) do {                                \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  vector_base_type vsp8;                                                    \
  vector_base_type vsp16;                                                   \
  vector_base_type vsp32;                                                   \
  vector_base_type shiftarg;                                                \
  __asm__ volatile (                                                        \
    /* j * 8 (double word size)                                          */ \
    "sldi    %[t1],%[index],3\n\t"                                          \
    /* alias to W[j] location                                            */ \
    "add     %[t1],%[t1],%[wptr]\n\t"                                       \
    "addi    %[t0],%[t1],-128\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[w0],0,%[t0]\n\t"                                             \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]\n\t"                                            \
    "addi    %[t0],%[t1],-112\n\t"                                          \
    /* unaligned load                                                    */ \
    "lvx     %[w1],0,%[t0]\n\t"                                             \
    /* w0 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[w0],%[w1],%[w0],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-96\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w2],0,%[t0]\n\t"                                             \
    /* w1 = w[j-14] to w[j-13]                                           */ \
    "vperm   %[w1],%[w2],%[w1],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-80\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w3],0,%[t0]\n\t"                                             \
    /* w2 = w[j-12] to w[j-11]                                           */ \
    "vperm   %[w2],%[w3],%[w2],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-64\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w4],0,%[t0]\n\t"                                             \
    /* w3 = w[j-10] to w[j-9]                                            */ \
    "vperm   %[w3],%[w4],%[w3],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-48\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w5],0,%[t0]\n\t"                                             \
    /* w4 = w[j-8] to w[j-7]                                             */ \
    "vperm   %[w4],%[w5],%[w4],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-32\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w6],0,%[t0]\n\t"                                             \
    /* w5 = w[j-6] to w[j-5]                                             */ \
    "vperm   %[w5],%[w6],%[w5],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-16\n\t"                                           \
    /* unaligned load                                                    */ \
    "lvx     %[w7],0,%[t0]\n\t"                                             \
    /* w6 = w[j-4] to w[j-3]                                             */ \
    "vperm   %[w6],%[w7],%[w6],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],0\n\t"                                             \
    /* unaligned load                                                    */ \
    "lvx     %[vt0],0,%[t0]\n\t"                                            \
    /* w7 = w[j-2] to w[j-1]                                             */ \
    "vperm   %[w7],%[vt0],%[w7],%[vrb]\n\t"                                 \
    /* Load k values                                                     */ \
    /* j * 8 (word size)                                                 */ \
    "sldi    %[t1],%[index],3\n\t"                                          \
    /* alias to k[j] location                                            */ \
    "add     %[t1],%[t1],%[kptr]\n\t"                                       \
    "addi    %[t0],%[t1],-128\n\t"                                          \
    /* load k[j-16] to k[j-15] to vector                                 */ \
    "lvx     %[k0],0,%[t0]\n\t"                                             \
    /* set vrb according to alignment                                    */ \
    "lvsr    %[vrb],0,%[t1]\n\t"                                            \
    "addi    %[t0],%[t1],-112\n\t"                                          \
    /* load k[j-14] to k[j-13] to vector                                 */ \
    "lvx     %[k1],0,%[t0]\n\t"                                             \
    /* k0 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k0],%[k1],%[k0],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-96\n\t"                                           \
    /* load k[j-12] to k[j-11] to vector                                 */ \
    "lvx     %[k2],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k1],%[k2],%[k1],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-80\n\t"                                           \
    /* load k[j-10] to k[j-9] to vector                                  */ \
    "lvx     %[k3],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k2],%[k3],%[k2],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-64\n\t"                                           \
    /* load k[j-8] to k[j-7] to vector                                   */ \
    "lvx     %[k4],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k3],%[k4],%[k3],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-48\n\t"                                           \
    /* load k[j-6] to k[j-5] to vector                                   */ \
    "lvx     %[k5],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k4],%[k5],%[k4],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-32\n\t"                                           \
    /* load k[j-4] to k[j-3] to vector                                   */ \
    "lvx     %[k6],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k5],%[k6],%[k5],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],-16\n\t"                                           \
    /* load k[j-2] to k[j-1] to vector                                   */ \
    "lvx     %[k7],0,%[t0]\n\t"                                             \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k6],%[k7],%[k6],%[vrb]\n\t"                                  \
    "addi    %[t0],%[t1],0\n\t"                                             \
    /* load k[j-2] to k[j-1] to vector                                   */ \
    "lvx     %[vt0],0,%[t0]\n\t"                                            \
    /* k1 = w[j-16] to w[j-15]                                           */ \
    "vperm   %[k7],%[vt0],%[k7],%[vrb]\n\t"                                 \
    /* Swap bytes                                                        */ \
    "vspltisw %[vsp16],8\n\t"                                               \
    "vspltisw %[shiftarg],1\n\t"                                            \
    "vsl      %[vsp16],%[vsp16], %[shiftarg]\n\t"                           \
    "vsl      %[vsp32],%[vsp16], %[shiftarg]\n\t"                           \
    "vspltish %[vsp8],8\n\t"                                                \
    "vrlh     %[w0],%[w0],%[vsp8]\n\t"                                      \
    "vrlh     %[w1],%[w1],%[vsp8]\n\t"                                      \
    "vrlh     %[w2],%[w2],%[vsp8]\n\t"                                      \
    "vrlh     %[w3],%[w3],%[vsp8]\n\t"                                      \
    "vrlh     %[w4],%[w4],%[vsp8]\n\t"                                      \
    "vrlh     %[w5],%[w5],%[vsp8]\n\t"                                      \
    "vrlh     %[w6],%[w6],%[vsp8]\n\t"                                      \
    "vrlh     %[w7],%[w7],%[vsp8]\n\t"                                      \
    "vrlw     %[w0],%[w0],%[vsp16]\n\t"                                     \
    "vrlw     %[w1],%[w1],%[vsp16]\n\t"                                     \
    "vrlw     %[w2],%[w2],%[vsp16]\n\t"                                     \
    "vrlw     %[w3],%[w3],%[vsp16]\n\t"                                     \
    "vrlw     %[w4],%[w4],%[vsp16]\n\t"                                     \
    "vrlw     %[w5],%[w5],%[vsp16]\n\t"                                     \
    "vrlw     %[w6],%[w6],%[vsp16]\n\t"                                     \
    "vrlw     %[w7],%[w7],%[vsp16]\n\t"                                     \
    "vrld     %[w0],%[w0],%[vsp32]\n\t"                                     \
    "vrld     %[w1],%[w1],%[vsp32]\n\t"                                     \
    "vrld     %[w2],%[w2],%[vsp32]\n\t"                                     \
    "vrld     %[w3],%[w3],%[vsp32]\n\t"                                     \
    "vrld     %[w4],%[w4],%[vsp32]\n\t"                                     \
    "vrld     %[w5],%[w5],%[vsp32]\n\t"                                     \
    "vrld     %[w6],%[w6],%[vsp32]\n\t"                                     \
    "vrld     %[w7],%[w7],%[vsp32]\n\t"                                     \
    /* calculate k+w                                                     */ \
    "vaddudm %[k0],%[k0],%[w0]\n\t"                                         \
    "vaddudm %[k1],%[k1],%[w1]\n\t"                                         \
    "vaddudm %[k2],%[k2],%[w2]\n\t"                                         \
    "vaddudm %[k3],%[k3],%[w3]\n\t"                                         \
    "vaddudm %[k4],%[k4],%[w4]\n\t"                                         \
    "vaddudm %[k5],%[k5],%[w5]\n\t"                                         \
    "vaddudm %[k6],%[k6],%[w6]\n\t"                                         \
    "vaddudm %[k7],%[k7],%[w7]\n\t"                                         \
    : /* output list                                                     */ \
      /* temporaries                                                     */ \
      [vsp8] "=&v" (vsp8),                                                  \
      [vsp16] "=&v" (vsp16),                                                \
      [vsp32] "=&v" (vsp32),                                                \
      [shiftarg] "=&v" (shiftarg),                                          \
      [t0] "=&b" (t0),                                                      \
      [t1] "=&b" (t1),                                                      \
      /* actual outputs                                                  */ \
      [vrb] "=v" ((_vRb)),                                                  \
      [vt0] "=v" ((vt0)),                                                   \
      [w0] "=v" ((_w0)),                                                    \
      [w1] "=v" ((_w1)),                                                    \
      [w2] "=v" ((_w2)),                                                    \
      [w3] "=v" ((_w3)),                                                    \
      [w4] "=v" ((_w4)),                                                    \
      [w5] "=v" ((_w5)),                                                    \
      [w6] "=v" ((_w6)),                                                    \
      [w7] "=v" ((_w7)),                                                    \
      [k0] "=v" ((_k0)),                                                    \
      [k1] "=v" ((_k1)),                                                    \
      [k2] "=v" ((_k2)),                                                    \
      [k3] "=v" ((_k3)),                                                    \
      [k4] "=v" ((_k4)),                                                    \
      [k5] "=v" ((_k5)),                                                    \
      [k6] "=v" ((_k6)),                                                    \
      [k7] "=v" ((_k7))                                                     \
    : /* input list                                                      */ \
      [index] "b" ((_j)),                                                   \
      [wptr] "b" ((_w)),                                                    \
      [kptr] "b" ((_k))                                                     \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

#define CALC_2W(_w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7, _kpw0, _kpw1,       \
                _j, _k) do {                                                \
  base_type t0;                                                             \
  base_type t1;                                                             \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  __asm__ volatile (                                                        \
    "sldi       %[t1],%[index],%[c1]\n\t"      /* j * 8 (doubleword size)*/ \
    "add        %[t0],%[t1],%[kptr]\n\t"       /* alias to k[j] location */ \
    "lvx        %[vt3],0,%[t0]\n\t"                                         \
    "addi       %[t0],%[t0],16\n\t"            /* alias to k[j+4] location*/\
    "lvsr       %[vt2],0,%[kptr]\n\t"                                       \
    "lvx        %[vt1],0,%[t0]\n\t"            /* treat unaligned case   */ \
    "vperm      %[vt3],%[vt1],%[vt3],%[vt2]\n\t"                            \
    "vsldoi     %[vt1],%[w1],%[w0],8\n\t"      /* vt1 = w[j-15] , w[j-14]*/ \
    "vsldoi     %[vt2],%[w5],%[w4],8\n\t"      /* vt2 = w[j-7] , w[j-6]  */ \
    /* vt1 = s0(w[j-15]) , s0(w[j-14])                                   */ \
    "vshasigmad %[vt1],%[vt1],0,0\n\t"                                      \
    /* vt4 = s1(w[j-2]) , s1(w[j-1])                                     */ \
    "vshasigmad %[vt4],%[w7],0,%[six1]\n\t"                                 \
    /* vt1 = s0(w[j-15]) + w[j-7] , s0(w[j-14]) + w[j-6]                 */ \
    "vaddudm    %[vt1],%[vt1],%[vt2]\n\t"                                   \
    /* vt0 = s1(w[j-2]) + w[j-16] , s1(w[j-1]) + w[j-15]                 */ \
    "vaddudm    %[vt0],%[vt4],%[w0]\n\t"                                    \
    /* vt1 = s0(w[j-15]) + w[j-7] + s1(w[j-2]) + w[j-16],  // w[j]       */ \
    /*       s0(w[j-14]) + w[j-6] + s1(w[j-1]) + w[j-15]   // w[j+1]     */ \
    "vaddudm    %[vt1],%[vt1],%[vt0]\n\t"                                   \
    /* Updating w0 to w7 to hold the "new previous" 16 values from w.    */ \
    "vor        %[w0],%[w1],%[w1]\n\t"                                      \
    "vor        %[w1],%[w2],%[w2]\n\t"                                      \
    "vor        %[w2],%[w3],%[w3]\n\t"                                      \
    "vor        %[w3],%[w4],%[w4]\n\t"                                      \
    "vor        %[w4],%[w5],%[w5]\n\t"                                      \
    "vor        %[w5],%[w6],%[w6]\n\t"                                      \
    "vor        %[w6],%[w7],%[w7]\n\t"                                      \
    "vor        %[w7],%[vt1],%[vt1]\n\t"                                    \
    /* store k + w to vt1 (2 values at once)                             */ \
    "vaddudm    %[vt1],%[vt1],%[vt3]\n\t"                                   \
    /* Deque values in vt1 to kpw0 and kpw1                              */ \
    "vmr        %[kpw0], %[vt1]\n\t"                                        \
    "vsldoi     %[kpw1], %[vt1],%[vt1], 8\n\t"                              \
  : /* output list                                                       */ \
    /* temporaries                                                       */ \
    [t0] "=&b" (t0),                                                        \
    [t1] "=&b" (t1),                                                        \
    [vt0] "=&v" (vt0),                                                      \
    [vt1] "=&v" (vt1),                                                      \
    [vt2] "=&v" (vt2),                                                      \
    [vt3] "=&v" (vt3),                                                      \
    [vt4] "=&v" (vt4),                                                      \
    /* outputs/inputs                                                    */ \
    [w0] "+v" ((_w0)),                                                      \
    [w1] "+v" ((_w1)),                                                      \
    [w2] "+v" ((_w2)),                                                      \
    [w3] "+v" ((_w3)),                                                      \
    [w4] "+v" ((_w4)),                                                      \
    [w5] "+v" ((_w5)),                                                      \
    [w6] "+v" ((_w6)),                                                      \
    [w7] "+v" ((_w7)),                                                      \
    /* actual outputs                                                    */ \
    [kpw0] "=v" ((_kpw0)),                                                  \
    [kpw1] "=v" ((_kpw1))                                                   \
  : /* input list                                                        */ \
    [index] "b" ((_j)),                                                     \
    [kptr] "b" ((_k)),                                                      \
    [c1] "i" (3),                                                           \
    [six1] "i" (0xf)                                                        \
  : /* clobber list                                                      */ \
    "memory"                                                                \
  ); } while (0)

#define UPDATE_SHA_STATE(_a, _b, _c, _d, _e, _f, _g, _h, _hptr) do {        \
  vector_base_type vt0;                                                     \
  vector_base_type vt1;                                                     \
  vector_base_type vt2;                                                     \
  vector_base_type vt3;                                                     \
  vector_base_type vt4;                                                     \
  vector_base_type vt5;                                                     \
  vector_base_type vt6;                                                     \
  vector_base_type vt7;                                                     \
  vector_base_type vt8;                                                     \
  vector_base_type vrb;                                                     \
  base_type rtmp0;                                                          \
  __asm__ volatile(                                                         \
      "lvsr    %[vrb],0,%[hptr]\n\t"                                        \
      "lvx     %[vt0],0,%[hptr]\n\t"        /* vt0 = {_h[0], _h[1]}      */ \
      "lvx     %[vt1],%[offs1],%[hptr]\n\t" /* vt1 = {_h[2], _h[3]}      */ \
      "vperm   %[vt0],%[vt1],%[vt0],%[vrb]\n\t"                             \
      "lvx     %[vt2],%[offs2],%[hptr]\n\t"                                 \
      "vperm   %[vt1],%[vt2],%[vt1],%[vrb]\n\t"                             \
      "xxmrgld %x[vt4],%x[b],%x[a]\n\t"     /* vt4 = {a, b}              */ \
      "xxmrgld %x[vt5],%x[d],%x[c]\n\t"     /* vt5 = {c, d}              */ \
      "xxmrgld %x[vt6],%x[f],%x[e]\n\t"     /* vt6 = {e, f}              */ \
      "xxmrgld %x[vt7],%x[h],%x[g]\n\t"     /* vt7 = {g, h}              */ \
      "lvx     %[vt3],%[offs3],%[hptr]\n\t" /* vt3 = {_h[6], _h[7]}      */ \
      "vperm   %[vt2],%[vt3],%[vt2],%[vrb]\n\t"                             \
      "lvx     %[vt8],%[offs4],%[hptr]\n\t"                                 \
      "vperm   %[vt3],%[vt8],%[vt3],%[vrb]\n\t"                             \
      "vaddudm %[vt0],%[vt0],%[vt4]\n\t"    /* vt0 = {_h[0]+a, _h[1]+b}  */ \
      "vaddudm %[vt1],%[vt1],%[vt5]\n\t"    /* vt1 = {_h[2]+c, _h[3]+d}  */ \
      "vaddudm %[vt2],%[vt2],%[vt6]\n\t"    /* vt2 = {_h[4]+e, _h[5]+f}  */ \
      "vaddudm %[vt3],%[vt3],%[vt7]\n\t"    /* vt3 = {_h[6]+g, _h[7]+h}  */ \
      "xxswapd %x[vt0],%x[vt0]\n\t"                                         \
      "stxvd2x %x[vt0],0,%[hptr]\n\t"                                       \
      "xxswapd %x[vt1],%x[vt1]\n\t"                                         \
      "stxvd2x %x[vt1],%[offs1],%[hptr]\n\t"                                \
      "xxswapd %x[vt2],%x[vt2]\n\t"                                         \
      "stxvd2x %x[vt2],%[offs2],%[hptr]\n\t"                                \
      "xxswapd %x[vt3],%x[vt3]\n\t"                                         \
      "stxvd2x %x[vt3],%[offs3],%[hptr]\n\t"                                \
      : /* output list                                                   */ \
      /* temporaries                                                     */ \
      [vt0] "=&v" (vt0),                                                    \
      [vt1] "=&v" (vt1),                                                    \
      [vt2] "=&v" (vt2),                                                    \
      [vt3] "=&v" (vt3),                                                    \
      [vt4] "=&v" (vt4),                                                    \
      [vt5] "=&v" (vt5),                                                    \
      [vt6] "=&v" (vt6),                                                    \
      [vt7] "=&v" (vt7),                                                    \
      [vrb] "=&v" (vrb),                                                    \
      [vt8] "=&v" (vt8),                                                    \
      [rtmp0] "=&b" ((rtmp0))                                               \
    : /* input list                                                      */ \
      [hptr] "b" ((_hptr)),                                                 \
      [a] "v" ((_a)),                                                       \
      [b] "v" ((_b)),                                                       \
      [c] "v" ((_c)),                                                       \
      [d] "v" ((_d)),                                                       \
      [e] "v" ((_e)),                                                       \
      [f] "v" ((_f)),                                                       \
      [g] "v" ((_g)),                                                       \
      [h] "v" ((_h)),                                                       \
      [offs1] "b" (16),                                                     \
      [offs2] "b" (32),                                                     \
      [offs3] "b" (48),                                                     \
      [offs4] "b" (64)                                                      \
    : /* clobber list                                                    */ \
      "memory"                                                              \
  ); } while (0)

void sha2_transform(base_type* _h, unsigned char* w) {
  vector_base_type a, b, c, d, e, f, g, h;
  vector int vRb;

  vector_base_type w0, w1, w2, w3, w4, w5, w6, w7;
  vector_base_type kplusw0, kplusw1, kplusw2, kplusw3, kplusw4, kplusw5,
                   kplusw6, kplusw7;
  vector_base_type kpw0, kpw1;

  int j = 16;

  LOAD_H_VEC(a, b, c, d, e, f, g, h, _h, vRb);

  // Load 16 elements from w out of the loop
  LOAD_W_PLUS_K(kplusw0, kplusw1, kplusw2, kplusw3, kplusw4, kplusw5, kplusw6,
                kplusw7, w0, w1, w2, w3, w4, w5, w6, w7, vRb, j, k, w);

  // Loop unrolling, from 0 to 15
  DEQUE(kplusw0, kpw1);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kplusw0);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);

  DEQUE(kplusw1, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kplusw1);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);

  DEQUE(kplusw2, kpw1);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kplusw2);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);

  DEQUE(kplusw3, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kplusw3);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);

  DEQUE(kplusw4, kpw1);
  SHA2_ROUND(a, b, c, d, e, f, g, h, kplusw4);
  SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);

  DEQUE(kplusw5, kpw1);
  SHA2_ROUND(g, h, a, b, c, d, e, f, kplusw5);
  SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);

  DEQUE(kplusw6, kpw1);
  SHA2_ROUND(e, f, g, h, a, b, c, d, kplusw6);
  SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);

  DEQUE(kplusw7, kpw1);
  SHA2_ROUND(c, d, e, f, g, h, a, b, kplusw7);
  SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);


  // From 16 to W_SIZE (80) in 8 steps
  while (j < W_SIZE) {
    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, k);
    SHA2_ROUND(a, b, c, d, e, f, g, h, kpw0);
    SHA2_ROUND(h, a, b, c, d, e, f, g, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, k);
    SHA2_ROUND(g, h, a, b, c, d, e, f, kpw0);
    SHA2_ROUND(f, g, h, a, b, c, d, e, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, k);
    SHA2_ROUND(e, f, g, h, a, b, c, d, kpw0);
    SHA2_ROUND(d, e, f, g, h, a, b, c, kpw1);
    j += 2;

    CALC_2W(w0, w1, w2, w3, w4, w5, w6, w7, kpw0, kpw1, j, k);
    SHA2_ROUND(c, d, e, f, g, h, a, b, kpw0);
    SHA2_ROUND(b, c, d, e, f, g, h, a, kpw1);
    j += 2;
  }

  UPDATE_SHA_STATE(a, b, c, d, e, f, g, h, _h);
}
