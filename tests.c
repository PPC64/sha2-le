#include "sha2.h"

static int error_occured = 0;

void print_error(int line) {
  printf("ERROR on line %d!\n", line);
  error_occured = 1;
}

void print_error_arg(int line, size_t arg) {
  printf("ERROR on line %d! Output: %lu\n", line, arg);
  error_occured = 1;
}

int main () {

#if SHA_BITS==256
  size_t size, padded_size;
  size =  3;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size !=  64) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size !=  64) print_error_arg(__LINE__, padded_size);

  size = 63;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);

  size = 64;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);

  size = 65;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);
  // testing around 5G
  size = 5368709119;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);

  size = 5368709120;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);

  size = 5368709121;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709184) print_error_arg(__LINE__, padded_size);
#endif

#if SHA_BITS==512
  size_t size, padded_size;
  size =  3;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 128) print_error_arg(__LINE__, padded_size);

  size = 127;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);

  size = 128;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);

  size = 129;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 256) print_error_arg(__LINE__, padded_size);

  // testing around 5G
  size = 5368709119;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);

  size = 5368709120;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);

   size = 5368709121;
  padded_size = calculate_padded_msg_size(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
  padded_size = calculate_padded_msg_size_FIPS_180_4(size);
  if (padded_size != 5368709248) print_error_arg(__LINE__, padded_size);
#endif

  return error_occured;
}
