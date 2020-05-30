#ifndef CORE_ENCODING_KOREAN_H
#define CORE_ENCODING_KOREAN_H

#include <stdint.h>

void encoding_korean_init(void);

void encoding_korean_to_utf8(const uint8_t *input, char *output, int output_length);

void encoding_korean_from_utf8(const char *input, uint8_t *output, int output_length);

#endif // CORE_ENCODING_KOREAN_H
