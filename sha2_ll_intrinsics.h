#ifndef _PPC64_LE_SHA2_LL_INTRINSICS_H_
#define _PPC64_LE_SHA2_LL_INTRINSICS_H_

#if LOW_LEVEL != 1
#error "The sha2_ll_intrinsics.h header should only be included on LOW_LEVEL == 1"
#endif

#include <altivec.h>

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

#define a 0
#define b 1
#define c 2
#define d 3
#define e 4
#define f 5
#define g 6
#define h 7

void static inline sha2_round(base_type* _ha, base_type kplusw) {

  vector_base_type bsigma;
  base_type tmp1, tmp2;

  bsigma[0] = _ha[a];
  bsigma[1] = _ha[e];
#if SHA_BITS == 256
  vector_base_type chv, va, vb, vc, ve, vf, vg, majv;
  va[0] = _ha[a];
  vb[0] = _ha[b];
  vc[0] = _ha[c];
  ve[0] = _ha[e];
  vf[0] = _ha[f];
  vg[0] = _ha[g];
  chv = vec_sel(vg, vf, ve);
  bsigma = __builtin_crypto_vshasigmaw(bsigma, 1, 0xE);
  tmp1 =  _ha[h] + bsigma[1] + chv[0] + kplusw;
  majv = vec_vxor(va, vb);
  majv = vec_sel(vb, vc, majv);
  tmp2 = bsigma[0] + majv[0];
#elif SHA_BITS == 512
  vector_base_type v0 = vec_and( (vector_base_type){bsigma[1], bsigma[0]},
      (vector_base_type){_ha[f], _ha[b]} );

  vector_base_type v1 = vec_and( (vector_base_type){~bsigma[1],bsigma[0]},
      (vector_base_type){_ha[g],_ha[c]} );


  bsigma = __builtin_crypto_vshasigmad(bsigma, 1, 0xD);


  vector_base_type v2 = (vector_base_type){0,(_ha[b] & _ha[c])} ^ v0 ^ v1;
  tmp1 = _ha[h] + bsigma[1] + v2[0] +kplusw;
  tmp2 = bsigma[0] + v2[1];
#endif

  _ha[h] = _ha[g];
  _ha[g] = _ha[f];
  _ha[f] = _ha[e];
  _ha[e] = _ha[d] + tmp1;
  _ha[d] = _ha[c];
  _ha[c] = _ha[b];
  _ha[b] = _ha[a];
  _ha[a] = tmp1 + tmp2;
}

void sha2_transform(base_type* _h, base_type* w) {
  int i;
  base_type _ha[8]  __attribute__ ((aligned (16))) =
    { _h[a],_h[b],_h[c],_h[d],_h[e],_h[f],_h[g],_h[h] };

  // Loop unrolling, from 0 to 15
  for (i = 0; i < 16; i++) {
    sha2_round(_ha, k[i]+w[i]);
  }

#if SHA_BITS == 256
  // Load 16 elements from w out of the loop
  vector_base_type v0,v1,v2,v3,v4,v5,sigma0,
                   sigma1,sigma12, result;
  v0[0] = w[i-16];
  v0[1] = w[i-15];
  v0[2] = w[i-14];
  v0[3] = w[i-13];

  v1[0] = w[i-12];
  v1[1] = w[i-11];
  v1[2] = w[i-10];
  v1[3] = w[i-9];

  v2[0] = w[i-8];
  v2[1] = w[i-7];
  v2[2] = w[i-6];
  v2[3] = w[i-5];

  v3[0] = w[i-4];
  v3[1] = w[i-3];
  v3[2] = w[i-2];
  v3[3] = w[i-1];
  // BEWARE!!! we do *not* have a strong guarantee that the v register won't
  // get dirty in between this two __asm__ calls.
  // As of now, this is just a quick experiment and SHOULD be fixed.

  // From 16 to W_SIZE (64)
  for (; i < W_SIZE; i=i+4) {
    v4 = (vector_base_type) { v0[1],v0[2],v0[3],v1[0] };
    v5 = (vector_base_type) { v2[1],v2[2],v2[3],v3[0] };
    sigma0 = __builtin_crypto_vshasigmaw(v4, 0, 0x0);
    sigma1 = __builtin_crypto_vshasigmaw(v3, 0, 0xf);

    result = (vector_base_type){ sigma0[0] + sigma1[2] + v0[0] + v5[0],
       sigma0[1] + sigma1[3] + v0[1] + v5[1],
       sigma0[2] + sigma1[2] + v0[2] + v5[2],
       sigma0[3] + sigma1[3] + v0[3] + v5[3] };

    sigma12 = __builtin_crypto_vshasigmaw(result, 0, 0xf);
    result = (vector_base_type){sigma1[2],sigma1[3],sigma12[0],sigma12[1]}+
      sigma0 + v0 +v5;

    v0 = v1;
    v1 = v2;
    v2 = v3;
    v3 = result;

    sha2_round(_ha, result[0]+k[i+0] );
    sha2_round(_ha, result[1]+k[i+1] );
    sha2_round(_ha, result[2]+k[i+2] );
    sha2_round(_ha, result[3]+k[i+3] );
  }

#else
  // Load 16 elements from w out of the loop
  vector_base_type v0,v1,v2,v3,v4,v5,v6,v7,sigma0,
                   sigma1, result;
  v0[0] = w[i-16];
  v0[1] = w[i-15];

  v1[0] = w[i-14];
  v1[1] = w[i-13];

  v2[0] = w[i-12];
  v2[1] = w[i-11];

  v3[0] = w[i-10];
  v3[1] = w[i-9];

  v4[0] = w[i-8];
  v4[1] = w[i-7];

  v5[0] = w[i-6];
  v5[1] = w[i-5];

  v6[0] = w[i-4];
  v6[1] = w[i-3];

  v7[0] = w[i-2];
  v7[1] = w[i-1];

  // From 16 to W_SIZE (64)
  for (; i < W_SIZE; i=i+2) {
    sigma0 = __builtin_crypto_vshasigmad((vector_base_type){v0[1],v1[0]},
        0, 0x0);
    sigma1 = __builtin_crypto_vshasigmad((vector_base_type){v7[0],v7[1]},
        0, 0xf);
    result = sigma0 + sigma1 + v0 + (vector_base_type){v4[1], v5[0]};

    v0 = v1;
    v1 = v2;
    v2 = v3;
    v3 = v4;
    v4 = v5;
    v5 = v6;
    v6 = v7;
    v7 = result;

    sha2_round(_ha, result[0]+k[i+0]);
    sha2_round(_ha, result[1]+k[i+1]);
  }
#endif
  _h[0] += _ha[a];
  _h[1] += _ha[b];
  _h[2] += _ha[c];
  _h[3] += _ha[d];
  _h[4] += _ha[e];
  _h[5] += _ha[f];
  _h[6] += _ha[g];
  _h[7] += _ha[h];
}

#endif // _PPC64_LE_SHA2_LL_INTRINSICS_H_
