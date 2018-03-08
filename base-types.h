#ifndef PPC64_LE_BASE_TYPES_H
#define PPC64_LE_BASE_TYPES_H

#include <stdint.h>
#include <stdlib.h>

#if defined(__powerpc64__)
#include <altivec.h>
#else
// dummy define to avoid errors on x64 compilation
#define vector
#endif

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

#endif // PPC64_LE_BASE_TYPES_H
