#ifndef _PPC64_LE_SHA2_LL_INTRINSICS_H_
#define _PPC64_LE_SHA2_LL_INTRINSICS_H_

#if LOW_LEVEL != 1
#error "The sha2_ll_intrinsics.h header should only be included on LOW_LEVEL == 1"
#endif

void calculate_higher_values(base_type *w) {
  for (int j = 16; j < W_SIZE; j++) {
    vector_base_type v;
    v[0] = w[j-15];
    v[1] = w[j-2];
#if SHA_BITS == 256
    v = __builtin_crypto_vshasigmaw(v, 0, 0x2);
#elif SHA_BITS == 512
    v = __builtin_crypto_vshasigmad(v, 0, 0xD);
#endif
    base_type s1, s0;
    s0 = v[0];
    s1 = v[1];
    w[j] = w[j-16] + s0 + w[j-7] + s1;
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

    base_type ch = calc_ch(e, f, g);
    base_type temp1 = h + S1 + ch + k[i] + w[i];
    base_type maj = calc_maj(a, b, c);
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
