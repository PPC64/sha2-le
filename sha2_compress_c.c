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

#define Ch(e, f, g) (((e) & (f)) ^ ((~e) & (g)))
#define Maj(a, b, c) (((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))

#if SHA_BITS == 256
static const base_type S0_args[] = { 2, 13, 22};
static const base_type S1_args[] = { 6, 11, 25};
static const base_type s0_args[] = { 7, 18,  3};
static const base_type s1_args[] = {17, 19, 10};
#elif SHA_BITS == 512
static const base_type S0_args[] = {28, 34, 39};
static const base_type S1_args[] = {14, 18, 41};
static const base_type s0_args[] = { 1,  8,  7};
static const base_type s1_args[] = {19, 61,  6};
#endif

void static inline sha2_round(base_type* a, base_type* b, base_type* c,
                              base_type* d, base_type* e, base_type* f,
                              base_type* g, base_type* h, base_type k,
                              base_type w) {

  base_type tmp1, tmp2;
  base_type ch = Ch(*e,*f,*g);
  base_type maj = Maj(*a,*b,*c);
  base_type bigsigma1 = BIGSIGMA1(*e);
  base_type bigsigma0 = BIGSIGMA0(*a);
  tmp1 = *h + bigsigma1 + ch + k + w;
  tmp2 = bigsigma0 + maj;

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
  base_type a, b, c, d, e, f, g, h;
  int i;

  a = _h[0];
  b = _h[1];
  c = _h[2];
  d = _h[3];
  e = _h[4];
  f = _h[5];
  g = _h[6];
  h = _h[7];

  // Loop unrolling, from 0 to 15
  for (i = 0; i < 16; i++) {
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[i], w[i]);
  }

  // From 16 to W_SIZE (64)
  for (; i < W_SIZE; i++) {
    w[i] = w[i-16] + SIGMA0(w[i-15]) + w[i-7] + SIGMA1(w[i-2]);
    sha2_round(&a, &b, &c, &d, &e, &f, &g, &h, k[i], w[i]);
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
