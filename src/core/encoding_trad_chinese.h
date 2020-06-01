#ifndef CORE_ENCODING_TRAD_CHINESE_H
#define CORE_ENCODING_TRAD_CHINESE_H

#include <stdint.h>

void encoding_trad_chinese_init(void);

void encoding_trad_chinese_to_utf8(const uint8_t *input, char *output, int output_length);

void encoding_trad_chinese_from_utf8(const char *input, uint8_t *output, int output_length);

int encoding_trad_chinese_big5_to_image_id(int big5);

#endif // CORE_ENCODING_TRAD_CHINESE_H
