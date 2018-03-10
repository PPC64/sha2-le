#ifndef PPC64_LE_SHA2_COMPRESS_H
#define PPC64_LE_SHA2_COMPRESS_H

#include "base-types.h"

#if SHA_BITS == 256

// sha256_compress is found on sha256_compress.s
extern void sha256_compress_ppc(base_type* _h, const unsigned char* w_in, const base_type *k);

#elif SHA_BITS == 512

void sha512_compress(base_type* _h, const unsigned char* w, const base_type *k);

#endif

#endif // PPC64_LE_SHA2_COMPRESS_H
