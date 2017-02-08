// Define these macros when compiling (See Makefile):
//
// SHA_BITS      256 or 512
// USE_HW_VECTOR 0 or 1

// To compile with libcrypto, define LIBCRYPTO

#ifndef LIBCRYPTO
 #include "sha2.h"
#else // LIBCRYPTO defined
 #include <openssl/sha.h>
#endif //ifndef LIBCRYPTO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

int main (int argc, char *argv[]) {
  char *filename = argv[1];
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    fprintf(stderr, "Error while open %s. %s\n", argv[1],
    strerror(errno));
    return errno;
  }

  // Get file size
  struct stat st;

  if (stat(argv[1], &st) != 0) {
    fprintf(stderr, "Cannot determine size of %s: %s\n", argv[1],
    strerror(errno));
    fclose(file);
    return errno;
  }

#ifndef LIBCRYPTO
  // Padding. padded_size is total message bytes including pad bytes.
  size_t padded_size = calculate_padded_msg_size(st.st_size);

  // Save file in a input buffer.
  unsigned char* input = (unsigned char *) calloc(padded_size,
                                                  sizeof(unsigned char));
#else // LIBCRYPTO defined
  unsigned char* input = (unsigned char *) calloc(st.st_size,
                                                  sizeof(unsigned char));
#endif
  if (input == NULL) {
    fprintf(stderr, "%s\n.", strerror(errno));
    return errno;
  }

  if (fread(input, sizeof(unsigned char), st.st_size, file) != st.st_size) {
    fprintf(stderr, "Read error on file %s. %s\n", argv[1],
    strerror(errno));
    fclose(file);
    return errno;
  }

#ifdef LIBCRYPTO
  unsigned char md[SHA_BITS/8];
 #if SHA_BITS == 256
  SHA256(input, st.st_size, md);
  for (int i = 0; i < SHA_BITS/8; i++) {
    printf("%02x", md[i]);
  }
  printf("\n");
 #elif SHA_BITS == 512
  SHA512(input, st.st_size, md);
  for (int i = 0; i < SHA_BITS/8; i++) {
    printf("%02x", md[i]);
  }
  printf("\n");
 #else // SHA_BITS != 256 && SHA_BITS != 512
  #error "SHA_BITS should be 256 or 512"
 #endif //if SHA_BITS == 256
  fclose(file);
  return 0;
#else //LIBCRYPTO not defined
  fclose(file);
  return sha2(input, st.st_size, padded_size);
#endif
}
