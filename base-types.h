#ifndef _PPC64_LE_BASE_TYPES_H_
#define _PPC64_LE_BASE_TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <altivec.h>

#if SHA_BITS == 256
typedef unsigned int base_type;
const base_type k[64];
#elif SHA_BITS == 512
typedef unsigned long long base_type;
const base_type k[80];
#endif // SHA_BITS

base_type _h[8];
const size_t W_SIZE;

static const size_t base_type_size = sizeof(base_type);
typedef vector base_type vector_base_type;

#endif // _PPC64_LE_BASE_TYPES_H_
