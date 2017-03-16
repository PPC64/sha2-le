#include "sha2_compress.h"

/*
 * As SHA2_ROUND is only defining the new elements and the rest is being
 * handled outside the macro, the new elements that should be 'a' and 'e' are
 * actually the previous ones.
 *
 * This approach avoids moving registers around. The drawback is that it'll
 * generate a big code due to different registers being used.
 */
// sha256_compress is found on sha256_compress.s
extern void sha256_compress(uint32_t *STATE, const uint8_t *input, const uint32_t *k);

void sha2_transform(base_type* _h, unsigned char* w) {
  sha256_compress(_h, w, k);
}

