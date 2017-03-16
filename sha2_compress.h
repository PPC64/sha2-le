#ifndef _PPC64_LE_SHA2_COMPRESS_H_
#define _PPC64_LE_SHA2_COMPRESS_H_

#include "base-types.h"

#if SHA_BITS == 256

// sha256_compress is found on sha256_compress.s
extern void sha256_compress(base_type* _h, const unsigned char* w_in, const base_type *k);

#elif SHA_BITS == 512

void sha512_compress(base_type* _h, const unsigned char* w, const base_type *k);

#endif

#endif // _PPC64_LE_SHA2_COMPRESS_H_
