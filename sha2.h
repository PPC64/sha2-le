#ifndef _PPC64_LE_SHA2_H_
#define _PPC64_LE_SHA2_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/stat.h>

#include "base-types.h"

#if SHA_BITS == 256
base_type _h[8] = {
  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
  0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
#elif SHA_BITS == 512
base_type _h[8] = {
  0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
  0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
  0x510e527fade682d1, 0x9b05688c2b3e6c1f,
  0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};
#endif

#if (LOW_LEVEL == 2 || LOW_LEVEL == 1) && !defined(__powerpc64__)
	#error "HW vector only implemented for powerpc64"
#endif

#if LOW_LEVEL == 2
#include "sha2_ll_asm.h"
#elif LOW_LEVEL == 1
#include "sha2_ll_intrinsics.h"
#else
#include "sha2_no_ll.h"
#endif


// Directly way to calculate padding plus 8 bytes appended at the end
static inline size_t calculate_padded_msg_size(size_t size) {
  // if 0x80 (1 byte) prepended to BLOCK_SIZE bytes of message doesn't fit
  // the next multiple of BLOCK_SIZE, append another block to align it to the
  // next multiple of BLOCK_SIZE.
  return (BLOCK_SIZE - (size % BLOCK_SIZE)) + ((size % BLOCK_SIZE >
    BLOCK_SIZE - 9) ? BLOCK_SIZE : 0) + size;
}

// Message M with lenght l
// 64-bits (sha256) or 128-bits (sha 512) block
// lets S = SHA_BITS (256, 512...)
// Append 1 to M and then k zero bits where k = (S*2-(l+1)) mod S*2
// then append (S/4) bits block to M with l value represented in binary
// Return result in bytes dividing all by CHAR_BIT
size_t calculate_padded_msg_size_FIPS_180_4(size_t size) {
  uint32_t block_size = SHA_BITS * 2;
  int64_t k = (block_size - (block_size/8)) - ((size * CHAR_BIT) + 1);

  // in fact % operator is more like a remainder operator not really module
  // and not work with negative numbers. To do that correctly we must add
  // the result r = a % b with abs(b) if r < 0.
  k %= block_size;
  k = (k < 0) ? k + block_size : k;

  return size + (((k + 1) + (block_size/8)) / CHAR_BIT);
}

void swap_bytes(char *input, char *output, size_t size) {
  size_t size_in_words = size / base_type_size;

  for (size_t i = 0; i < size_in_words; i++) {
    base_type *input_cast = (base_type*)input+i;
    base_type *output_cast = (base_type*)output+i;

#if __BYTE_ORDER__ ==  __ORDER_LITTLE_ENDIAN__

    *output_cast =
#if SHA_BITS == 256
    (*input_cast & 0xFF000000) >> 24 |
    (*input_cast & 0x00FF0000) >>  8 |
    (*input_cast & 0x0000FF00) <<  8 |
    (*input_cast & 0x000000FF) << 24;
#elif SHA_BITS == 512
    (*input_cast & 0xFF00000000000000ULL) >> 56 |
    (*input_cast & 0x00FF000000000000ULL) >> 40 |
    (*input_cast & 0x0000FF0000000000ULL) >> 24 |
    (*input_cast & 0x000000FF00000000ULL) >>  8 |
    (*input_cast & 0x00000000FF000000ULL) <<  8 |
    (*input_cast & 0x0000000000FF0000ULL) << 24 |
    (*input_cast & 0x000000000000FF00ULL) << 40 |
    (*input_cast & 0x00000000000000FFULL) << 56;
#endif // SHA_BITS

#else
#error "Only for little endian"
#endif

  }
}

// TODO(rcardoso): I am not sure how this algorithm will work for SHA512. How
// this write the high 64 bits of length?
void write_size(char *input, size_t size, size_t position) {
  base_type* total_size = (base_type*)&input[position];
  // Undefined for SHA512. Right shift count >= width of type (uint64_t)
  #if SHA_BITS == 256
  *total_size = (base_type)((size * 8) >> 32); // higher bits
  #endif
  *(++total_size) = (base_type)size * 8; // lower bits
}

int sha2(char *input, size_t size, size_t padded_size) {

  // Concatenate '1' to input.
  input[size] = (char)(1 << 7);

  // Swap bytes due to endianess .
  char* input_swapped = (char *) calloc(padded_size, sizeof(char));
  if (input_swapped == NULL) {
    fprintf(stderr, "%s\n.", strerror(errno));
    return errno;
  }
  swap_bytes(input, input_swapped, padded_size);

  // write total message size at the end (2 base_types).
  write_size(input_swapped, size, padded_size - 2 * base_type_size);

  // Sha compression process.
  for (size_t i = 0; i < padded_size; i = i + BLOCK_SIZE) {
    base_type w[W_SIZE] __attribute__ ((aligned (16)));
    memcpy(w, input_swapped + i, 16 * sizeof(base_type));
    sha2_transform(_h, w);
  }

  printf(
#if SHA_BITS == 256
  "%08x%08x%08x%08x%08x%08x%08x%08x\n",
#elif SHA_BITS == 512
  "%016lx%016lx%016lx%016lx%016lx%016lx%016lx%016lx\n",
#endif
   _h[0],_h[1],_h[2],_h[3],_h[4],_h[5],_h[6],_h[7]);
  return 0;
}

#endif // _PPC64_LE_BASE_TYPES_H_
