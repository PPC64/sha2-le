#ifndef _PPC64_LE_SHA2_LL_INTRINSICS_H_
#define _PPC64_LE_SHA2_LL_INTRINSICS_H_

#if LOW_LEVEL != 1
#error "The sha2_ll_intrinsics.h header should only be included on LOW_LEVEL == 1"
#endif

#include "base-types.h"

void calculate_higher_values(base_type *w) {
  vector_base_type s; // small sigma vector
#if SHA_BITS == 256
  // Expand two message blocks per loop cycle
  for (int t = 16; t < W_SIZE; t += 2) {
    //sig0  = w[t-15];
    //sig1  = w[t-2];
    //sig0' = w[t-14];
    //sig1' = w[t-1];
    s = (vector_base_type) { w[t-15], w[t-2], w[t-14], w[t-1] };
    s = __builtin_crypto_vshasigmaw(s, 0, 0xA);
    w[t]   = s[1] + w[t-7] + s[0] + w[t-16];
    w[t+1] = s[3] + w[t-6] + s[2] + w[t-15];
    // Wt is added to Kt in compression function we can move this operation to
    // message scheduler; This can be done since both Wt and Kt are available
    // and there's no dependency with the others operands. Since we already
    // have the operands avaliable here we can sum avoiding load those
    // operands again on compression phase.
    // TODO(rcardoso): we can try to sum the two values with one operation.
    w[t-16] += k[t-16];
    w[t-15] += k[t-15];
#elif SHA_BITS == 512
for (int t = 16; t < W_SIZE; t++) {
    // sig0 = w[t-15];
    // sig1 = w[t-2];
    s = (vector_base_type) {  w[t-15], w[t-2] };
    s = __builtin_crypto_vshasigmad(s, 0, 0xD);
    w[t]   = s[1] + w[t-7] + s[0] + w[t-16];
    // Wt + Kt
    w[t-16] += k[t-16];
#endif
  }
  // Continue to sum Kt to Wt. Note that we can fallthrough from previous loop,
  // unroll that loop and do the sums in parallel using vector add operations.
  for (int i=W_SIZE-16; i < W_SIZE; i++) {
    w[i] += k[i];
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
    vector_base_type v;
    v[0] = a;
    v[1] = e;
#if SHA_BITS == 256
    v = __builtin_crypto_vshasigmaw(v, 1, 0xE);
#elif SHA_BITS == 512
    v = __builtin_crypto_vshasigmad(v, 1, 0xD);
#endif
    base_type S1, S0;
    S0 = v[0];
    S1 = v[1];

    base_type ch = (e & f) ^ (~e & g);
    base_type temp1 = h + S1 + ch + w[i];
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

#endif // _PPC64_LE_SHA2_LL_INTRINSICS_H_
