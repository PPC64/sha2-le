#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


#include "base-types.h"
#include "sha2_compress.h"
#include "sha2_common.h"

#if SHA_BITS == 256

base_type _h[8] = {
  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
  0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

const base_type k[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
  0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
  0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
  0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
  0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
  0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#elif SHA_BITS == 512

base_type _h[8] = {
  0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
  0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
  0x510e527fade682d1, 0x9b05688c2b3e6c1f,
  0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

const base_type k[80] = {
  0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f,
  0xe9b5dba58189dbbc, 0x3956c25bf348b538, 0x59f111f1b605d019,
  0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242,
  0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
  0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235,
  0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3,
  0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 0x2de92c6f592b0275,
  0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
  0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f,
  0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725,
  0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc,
  0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
  0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6,
  0x92722c851482353b, 0xa2bfe8a14cf10364, 0xa81a664bbc423001,
  0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218,
  0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
  0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99,
  0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb,
  0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc,
  0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
  0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915,
  0xc67178f2e372532b, 0xca273eceea26619c, 0xd186b8c721c0c207,
  0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba,
  0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
  0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc,
  0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a,
  0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

#else
#error "Invalid SHA_BITS"
#endif

// Direct way to calculate padding plus 8 bytes appended at the end
size_t calculate_padded_msg_size(size_t size) {
  // if 0x80 (1 byte) prepended to BLOCK_SIZE bytes of message doesn't fit
  // the next multiple of BLOCK_SIZE, append another block to align it to
  // the next multiple of BLOCK_SIZE.
  return (BLOCK_SIZE - (size % BLOCK_SIZE)) + ((size % BLOCK_SIZE >
    BLOCK_SIZE - 9) ? BLOCK_SIZE : 0) + size;
}

// Message M with length l
// 64-bits (sha256) or 128-bits (sha 512) block
// let S = SHA_BITS (256, 512...)
// Append 1 to M and then k zero bits where k = (S*2-(l+1)) mod S*2
// then append (S/4) bits block to M with l value represented in binary
// Return result in bytes dividing all by CHAR_BIT
size_t calculate_padded_msg_size_FIPS_180_4(size_t size) {
  uint32_t block_size = SHA_BITS * 2;
  int64_t k = (block_size - (block_size/8)) - ((size * CHAR_BIT) + 1);

  // in fact % operator is more like a remainder operator not really module
  // and does not work with negative numbers. To do that correctly we must
  // add the result r = a % b with abs(b) if r < 0.
  k %= block_size;
  k = (k < 0) ? k + block_size : k;

  return size + (((k + 1) + (block_size/8)) / CHAR_BIT);
}

void swap_bytes(unsigned char *input, unsigned char *output, size_t size) {
  size_t size_in_words = size / base_type_size;

  for (size_t i = 0; i < size_in_words; i++) {
    base_type *input_cast = (base_type*)input+i;
    base_type *output_cast = (base_type*)output+i;

#if __LITTLE_ENDIAN__
    *output_cast =
# if SHA_BITS == 256
      (*input_cast & 0xFF000000) >> 24 |
      (*input_cast & 0x00FF0000) >>  8 |
      (*input_cast & 0x0000FF00) <<  8 |
      (*input_cast & 0x000000FF) << 24;
# elif SHA_BITS == 512
      (*input_cast & 0xFF00000000000000ULL) >> 56 |
      (*input_cast & 0x00FF000000000000ULL) >> 40 |
      (*input_cast & 0x0000FF0000000000ULL) >> 24 |
      (*input_cast & 0x000000FF00000000ULL) >>  8 |
      (*input_cast & 0x00000000FF000000ULL) <<  8 |
      (*input_cast & 0x0000000000FF0000ULL) << 24 |
      (*input_cast & 0x000000000000FF00ULL) << 40 |
      (*input_cast & 0x00000000000000FFULL) << 56;
# endif // SHA_BITS
#else
    memcpy(output_cast, input_cast, sizeof(base_type));
#endif
  }
}

void write_size(unsigned char *input, size_t size, size_t position) {
  base_type* total_size = (base_type*)&input[position];
  const unsigned long long bit_size = size * 8;
  #if SHA_BITS == 256
  *total_size = (base_type)(bit_size >> 32); // higher bits
  #endif
  *(++total_size) = (base_type)bit_size; // lower bits
}

int sha2(unsigned char *input, size_t size, size_t padded_size) {

  // Concatenate '1' to input.
  input[size] = (unsigned char)(1 << 7);

  // write total message size at the end (2 base_types).
  write_size(input, size, padded_size - 2 * base_type_size);
  swap_bytes(input + padded_size - 2 * base_type_size,
      input + padded_size - 2 * base_type_size, 2 * base_type_size);

  // Sha compression process.
  for (size_t i = 0; i < padded_size; i = i + BLOCK_SIZE)
    sha2_compress(_h, input + i, k);

  printf(
#if SHA_BITS == 256
    "%08x%08x%08x%08x%08x%08x%08x%08x\n",
#elif SHA_BITS == 512
    "%016llx%016llx%016llx%016llx%016llx%016llx%016llx%016llx\n",
#endif
   _h[0],_h[1],_h[2],_h[3],_h[4],_h[5],_h[6],_h[7]);
  return 0;
}
