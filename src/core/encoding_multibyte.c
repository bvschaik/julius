#include "encoding_multibyte.h"

#include "core/image.h"

#include <stdint.h>

typedef struct {
    uint16_t image_id;
    uint16_t big5;
    uint16_t unicode;
} multibyte_entry;

// only define the characters that are actually used in the language files
static multibyte_entry entries[] = {
    { 4, 0xb943, 0x904a },
    { 5, 0xc0b8, 0x6232 },
    { 14, 0xc2f7, 0x96e2 },
    { 15, 0xb67d, 0x958b },
    { 51, 0xabd2, 0x5e1d },
    { 68, 0xad6e, 0x8981 },
    { 69, 0xc3b9, 0x7f85 },
    { 70, 0xb0a8, 0x99ac },
    { 71, 0xb0ea, 0x570b },
    { 72, 0xb6dc, 0x55ce },
    { 73, 0xa148, 0xff1f },
    { 80, 0xaaba, 0x7684 },
    { 118, 0xa175, 0x300C },
    { 119, 0xb3cd, 0x51f1 },
    { 120, 0xbcbb, 0x6492 },
    { 121, 0xa46a, 0x5927 },
    { 122, 0xa176, 0x300D },
    { 297, 0xa1d0, 0xff0d },
    { 782, 0xb751, 0x60f3 },
    { 1439, 0xa2bb, 0x2162 },
    { 0, 0, 0 }
};

int encoding_multibyte_big5_to_image_id(int big5)
{
    for (int i = 0; entries[i].image_id; i++) {
        if (entries[i].big5 == big5) {
            return entries[i].image_id;
        }
    }
    return -1;
}

static uint16_t to_unicode(const uint8_t byte1, const uint8_t byte2)
{
    int image_id = (byte1 & 0x7f) | ((byte2 & 0x7f) << 7);
    if (image_id < IMAGE_FONT_MULTIBYTE_CHINESE_MAX_CHARS) {
        for (int i = 0; entries[i].image_id; i++) {
            if (entries[i].image_id == image_id) {
                return entries[i].unicode;
            }
        }
    } else {
        // Input is not image ID but Big5 encoding: look up in table
        int big5 = byte1 << 8 | byte2;
        for (int i = 0; entries[i].image_id; i++) {
            if (entries[i].image_id == big5) {
                return entries[i].unicode;
            }
        }
    }
    return 0;
}

void encoding_multibyte_to_utf8(encoding_type encoding, const uint8_t *input, char *output, int output_length)
{
    const char *max_output = &output[output_length - 1];

    while (*input && output < max_output) {
        if (*input < 0x80) {
            *output = *input;
            ++output;
            ++input;
        } else {
            // multi-byte char
            // TODO differentiate between chinese and korean here
            uint16_t unicode = to_unicode(input[0], input[1]);
            if (unicode) {
                // Convert Unicode char to UTF-8
                if (unicode < 0x80) {
                    output[0] = (uint8_t)(unicode & 0x7f);
                    output++;
                } else if (unicode < 0x800) {
                    output[0] = (char)(((unicode >> 6) & 0x1F) | 0xC0);
                    output[1] = (char)((unicode & 0x3F) | 0x80);
                    output += 2;
                } else {
                    output[0] = (char)(((unicode >> 12) & 0x0F) | 0xE0);
                    output[1] = (char)(((unicode >> 6) & 0x3F) | 0x80);
                    output[2] = (char)((unicode & 0x3F) | 0x80);
                    output += 3;
                }
            }
            input += 2;
        }
    }
    *output = 0;
}
