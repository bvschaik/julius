#ifndef CORE_ENCODING_JAPANESE_H
#define CORE_ENCODING_JAPANESE_H

#include <stdint.h>

void encoding_japanese_init(void);

void encoding_japanese_to_utf8(const uint8_t *input, char *output, int output_length);

void encoding_japanese_from_utf8(const char *input, uint8_t *output, int output_length);

int encoding_japanese_sjis_to_image_id(uint8_t first, uint8_t second);

#endif // CORE_ENCODING_JAPANESE_H
