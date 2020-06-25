#include "encoding_simp_chinese.h"

#include "core/encoding.h"
#include "core/image.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

void encoding_simp_chinese_init(void)
{
    // Nothing to do here (yet)
}

void encoding_simp_chinese_to_utf8(const uint8_t *input, char *output, int output_length)
{
    const char *max_output = &output[output_length - 1];

    while (*input && output < max_output) {
        if (*input < 0x80) {
            *output = *input;
            ++output;
            ++input;
        } else {
            // multi-byte char: ignore for now
            input += 2;
        }
    }
    *output = 0;
}

void encoding_simp_chinese_from_utf8(const char *input, uint8_t *output, int output_length)
{
    const uint8_t *max_output = &output[output_length - 1];

    while (*input && output < max_output) {
        if ((*input & 0x80) == 0) {
            *output = *input;
            ++output;
            ++input;
        } else {
            // multi-byte char: ignore for now
            *output = '?';
            output++;
            input += encoding_get_utf8_character_bytes(*input);
        }
    }
    *output = 0;
}
