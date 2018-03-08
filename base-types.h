#ifndef PPC64_LE_BASE_TYPES_H
#define PPC64_LE_BASE_TYPES_H

#include <stdint.h>
#include <stdlib.h>

#if defined(__powerpc64__) || defined(__ALTIVEC__)
#include <altivec.h>
#else
// dummy define to avoid errors on x64 compilation
#define vector
#endif

#if SHA_BITS == 256
typedef unsigned int base_type;
typedef vector unsigned int vector_base_type;
extern const base_type h[8];
extern const base_type k[64];
static const size_t W_SIZE = 64;
#elif SHA_BITS == 512
typedef unsigned long long base_type;
typedef vector unsigned long long vector_base_type;
extern const base_type h[8];
extern const base_type k[80];
static const size_t W_SIZE = 80;
#endif // SHA_BITS

static const size_t base_type_size = sizeof(base_type);

#endif // PPC64_LE_BASE_TYPES_H
