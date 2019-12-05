#ifndef CORE_ENCODING_MULTIBYTE_H
#define CORE_ENCODING_MULTIBYTE_H

#include <stdint.h>

int encoding_multibyte_big5_to_image_id(int big5);

void encoding_multibyte_to_utf8(const uint8_t *input, char *output, int output_length);

#endif // CORE_ENCODING_MULTIBYTE_H
