#ifndef _PPC64_LE_SHA2_LL_INTRINSICS_H_
#define _PPC64_LE_SHA2_LL_INTRINSICS_H_

#if LOW_LEVEL != 1
#error "The sha2_ll_intrinsics.h header should only be included on LOW_LEVEL == 1"
#endif

#include "base-types.h"

void calculate_higher_values(base_type *w) {
  vector_base_type sigma; // small sigma vector
  vector_base_type w_vec, k_vec, kw_vec; // temp vectors to sum k+w
#if SHA_BITS == 256
  // Expand two message blocks per loop cycle
  for (int t = 16; t < W_SIZE; t += 2) {
    //sig0  = w[t-15];
    //sig1  = w[t-2];
    //sig0' = w[t-14];
    //sig1' = w[t-1];
    sigma = (vector_base_type) { w[t-15], w[t-2], w[t-14], w[t-1] };
    sigma = __builtin_crypto_vshasigmaw(sigma, 0, 0xA);
    w[t]   = sigma[1] + w[t-7] + sigma[0] + w[t-16];
    w[t+1] = sigma[3] + w[t-6] + sigma[2] + w[t-15];
    // Wt is added to Kt in compression function we can move this operation to
    // message scheduler; This can be done since both Wt and Kt are available
    // and there's no dependency with the others operands. Since we already
    // have the operands avaliable here we can sum avoiding load those
    // operands again on compression phase.
    // TODO(rcardoso): we can try to sum the two values with one operation.
    w[t-16] += k[t-16];
    w[t-15] += k[t-15];
  }

  // Continue to sum Kt to Wt. Sums the last sixteen values in parallel using
  // vector add operations (4x4). On asm code we can easily unroll this loop
  // avoiding branchs.
  for (int i=W_SIZE-16; i < W_SIZE; i += 4) {
    w_vec = (vector_base_type) {w[i], w[i+1], w[i+2], w[i+3] };
    k_vec = (vector_base_type) {k[i], k[i+1], k[i+2], k[i+3] };
    kw_vec = __builtin_vec_add(w_vec, k_vec);
    // TODO(rcardoso): Replace this for a vector store intrinsic
    w[i] = kw_vec[0];
    w[i+1] = kw_vec[1];
    w[i+2] = kw_vec[2];
    w[i+3] = kw_vec[3];
  }
#elif SHA_BITS == 512
  for (int t = 16; t < W_SIZE; t++) {
    // sig0 = w[t-15];
    // sig1 = w[t-2];
    sigma = (vector_base_type) {  w[t-15], w[t-2] };
    sigma = __builtin_crypto_vshasigmad(sigma, 0, 0xD);
    w[t]   = sigma[1] + w[t-7] + sigma[0] + w[t-16];
    // Wt + Kt
    w[t-16] += k[t-16];
  }
  // Sum the sixteen k to w (2x2)
  for (int i=W_SIZE-16; i < W_SIZE; i += 2) {
    w_vec = (vector_base_type) {w[i], w[i+1] };
    k_vec = (vector_base_type) {k[i], k[i+1] };
    kw_vec = __builtin_vec_add(w_vec, k_vec);
    // Replace this for a vector store instrinsic
    w[i] = kw_vec[0];
    w[i+1] = kw_vec[1];
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
