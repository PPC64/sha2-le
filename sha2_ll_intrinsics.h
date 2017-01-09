#ifndef _PPC64_LE_SHA2_LL_INTRINSICS_H_
#define _PPC64_LE_SHA2_LL_INTRINSICS_H_

#if LOW_LEVEL != 1
#error "The sha2_ll_intrinsics.h header should only be included on LOW_LEVEL == 1"
#endif

#include <altivec.h>

#include "base-types.h"

void static inline sha2_round(base_type* a, base_type* b, base_type* c,
                              base_type* d, base_type* e, base_type* f,
                              base_type* g, base_type* h, base_type k,
                              base_type w) {

  vector_base_type bsigma;
  base_type tmp1, tmp2;

  bsigma[0] = *a;
  bsigma[1] = *e;
#if SHA_BITS == 256
  bsigma = __builtin_crypto_vshasigmaw(bsigma, 1, 0xE);
#elif SHA_BITS == 512
  bsigma = __builtin_crypto_vshasigmad(bsigma, 1, 0xD);
#endif
  tmp1 =  *h + bsigma[1] + Ch(*e, *f, *g) + w + k;
  tmp2 = bsigma[0] + Maj(*a, *b, *c);

  *h = *g;
  *g = *f;
  *f = *e;
  *e = *d + tmp1;
  *d = *c;
  *c = *b;
  *b = *a;
  *a = tmp1 + tmp2;
}

void sha2_transform(base_type* _h, base_type* w) {
  vector_base_type sigma; // small sigma vector
  base_type a, b, c, d, e, f, g, h; // compression registers
  uint32_t t;

  a = _h[0];
  b = _h[1];
  c = _h[2];
  d = _h[3];
  e = _h[4];
  f = _h[5];
  g = _h[6];
  h = _h[7];

  for (t = 0; t < 16; t++) {
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[t], w[t]);
  }
#if SHA_BITS == 256
  // Expand two message blocks per loop cycle
  for (; t < W_SIZE; t += 2) {
    //sig0  = w[t-15];
    //sig1  = w[t-2];
    //sig0' = w[t-14];
    //sig1' = w[t-1];
    sigma = (vector_base_type) { w[t-15], w[t-2], w[t-14], w[t-1] };
    sigma = __builtin_crypto_vshasigmaw(sigma, 0, 0xA);
    w[t] = sigma[1] + w[t-7] + sigma[0] + w[t-16];
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[t], w[t]);
    w[t+1] = sigma[3] + w[t-6] + sigma[2] + w[t-15];
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[t+1], w[t+1]);
  }
#elif SHA_BITS == 512
  for (; t < W_SIZE; t++) {
    // TODO(rcardoso): for some reason fill vector using
    // sigma = (vector_base_type) { w[t-15], w[t-2] } give us wrong result
    // for sha512 when compile with -O3.
    sigma[0] = w[t-15];
    sigma[1] = w[t-2];
    sigma = __builtin_crypto_vshasigmad(sigma, 0, 0xD);
    w[t] = sigma[1] + w[t-7] + sigma[0] + w[t-16];
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[t], w[t]);
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
#endif // _PPC64_LE_SHA2_LL_INTRINSICS_H_
