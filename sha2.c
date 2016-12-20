// Define these macros when compiling (See Makefile):
//
// SHA_BITS      256 or 512
// USE_HW_VECTOR 0 or 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sha2.h"

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

  // Padding. padded_size is total message bytes including pad bytes.
  size_t padded_size = calculate_padded_msg_size(st.st_size);

  // Save file in a input buffer.
  char* input = (char *) malloc(padded_size);
  if (input == NULL) {
    fprintf(stderr, "%s\n.", strerror(errno));
    return errno;
  }

  if (fread(input, sizeof(char), st.st_size, file) != st.st_size) {
    fprintf(stderr, "Read error on file %s. %s\n", argv[1],
    strerror(errno));
    fclose(file);
    return errno;
  }

  fclose(file);

  return sha2(input, st.st_size, padded_size, _h);
}
