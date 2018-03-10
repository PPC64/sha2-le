#ifndef PPC64_LE_SHA2_H
#define PPC64_LE_SHA2_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "base-types.h"

// Direct way to calculate padding plus 8 bytes appended at the end
size_t calculate_padded_msg_size(size_t size);
size_t calculate_padded_msg_size_FIPS_180_4(size_t size);
void swap_bytes(unsigned char *input, unsigned char *output, size_t size);
void write_size(unsigned char *input, size_t size, size_t position);
int sha2(unsigned char *input, size_t size, size_t padded_size);

#if SHA_BITS == 256
#define BLOCK_SIZE 64
#define sha2_compress sha256_compress_ppc
#elif SHA_BITS == 512
#define BLOCK_SIZE 128
#define sha2_compress sha512_compress
#endif
#endif // PPC64_LE_SHA2_H

