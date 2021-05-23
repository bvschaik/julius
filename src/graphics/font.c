#include "font.h"

#include "core/encoding_trad_chinese.h"
#include "core/image.h"

static int image_y_offset_none(uint8_t c, int image_height, int line_height);
static int image_y_offset_default(uint8_t c, int image_height, int line_height);
static int image_y_offset_eastern(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_normal_small_plain(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_normal_colored(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_large_plain(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_large_black(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_large_brown(uint8_t c, int image_height, int line_height);
static int image_y_offset_cyrillic_normal_brown(uint8_t c, int image_height, int line_height);
static int image_y_offset_chinese(uint8_t c, int image_height, int line_height);
static int image_y_offset_korean(uint8_t c, int image_height, int line_height);

static const int CHAR_TO_FONT_IMAGE_DEFAULT[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3F, 0x40, 0x00, 0x00, 0x41, 0x00, 0x4A, 0x43, 0x44, 0x42, 0x46, 0x4E, 0x45, 0x4F, 0x4D,
    0x3E, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x48, 0x49, 0x00, 0x47, 0x00, 0x4B,
    0x00, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x00, 0x00, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x6E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00,
    0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x72, 0x70, 0x71, 0x71, 0x69, 0x83, 0x6D, 0x65, 0x74, 0x6A, 0x73, 0x73, 0x77, 0x75, 0x76, 0x76,
    0x00, 0x6C, 0x7A, 0x78, 0x79, 0x79, 0x7B, 0x00, 0x84, 0x7E, 0x7C, 0x7D, 0x6B, 0x33, 0x00, 0x68,
    0x53, 0x52, 0x54, 0x51, 0x51, 0x85, 0x67, 0x65, 0x57, 0x56, 0x58, 0x55, 0x5B, 0x5A, 0x5C, 0x59,
    0x00, 0x66, 0x5F, 0x5E, 0x60, 0x60, 0x5D, 0x00, 0x86, 0x63, 0x62, 0x64, 0x61, 0x19, 0x00, 0x19,
};

static const int CHAR_TO_FONT_IMAGE_EASTERN[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x20, 0x44, 0x6E, 0x00, 0x25, 0x00, 0x00, 0x00,
    0x00, 0x3F, 0x40, 0x00, 0x00, 0x41, 0x00, 0x4A, 0x43, 0x44, 0x42, 0x46, 0x4E, 0x45, 0x4F, 0x4D,
    0x3E, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x48, 0x49, 0x00, 0x47, 0x00, 0x4B,
    0x00, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x00, 0x63, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x65, 0x61, 0x56, 0x54, 0x51, 0x53, 0x01, 0x67, 0x81, 0x55, 0x57, 0x59, 0x6F, 0x5D, 0x69, 0x70,
    0x6A, 0x67, 0x6D, 0x60, 0x5D, 0x5F, 0x64, 0x63, 0x19, 0x7B, 0x6B, 0x00, 0x57, 0x00, 0x00, 0x58,
    0x52, 0x7F, 0x5E, 0x6C, 0x66, 0x69, 0x01, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x71,
    0x82, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59,
    0x72, 0x70, 0x71, 0x71, 0x69, 0x83, 0x6A, 0x65, 0x74, 0x6A, 0x6B, 0x73, 0x77, 0x75, 0x76, 0x76,
    0x00, 0x6D, 0x7A, 0x6E, 0x79, 0x79, 0x7B, 0x00, 0x84, 0x7E, 0x7C, 0x7D, 0x6B, 0x33, 0x00, 0x68,
    0x53, 0x52, 0x54, 0x51, 0x51, 0x85, 0x52, 0x65, 0x57, 0x56, 0x53, 0x55, 0x5B, 0x5A, 0x5C, 0x59,
    0x00, 0x55, 0x5F, 0x56, 0x60, 0x60, 0x5D, 0x00, 0x86, 0x63, 0x62, 0x64, 0x61, 0x19, 0x00, 0x19,
};

static const int CHAR_TO_FONT_IMAGE_CYRILLIC[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x20, 0x44, 0x6E, 0x00, 0x25, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x00, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
    0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
    0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
    0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E,
};

static const font_definition DEFINITIONS_DEFAULT[] = {
    {FONT_NORMAL_PLAIN,   0, 0, 6, 1, 11, image_y_offset_default},
    {FONT_NORMAL_BLACK, 134, 0, 6, 0, 11, image_y_offset_default},
    {FONT_NORMAL_WHITE, 268, 0, 6, 0, 11, image_y_offset_default},
    {FONT_NORMAL_RED,   402, 0, 6, 0, 11, image_y_offset_default},
    {FONT_LARGE_PLAIN,  536, 0, 8, 1, 23, image_y_offset_default},
    {FONT_LARGE_BLACK,  670, 0, 8, 0, 23, image_y_offset_default},
    {FONT_LARGE_BROWN,  804, 0, 8, 0, 24, image_y_offset_default},
    {FONT_SMALL_PLAIN,  938, 0, 4, 1, 9, image_y_offset_default},
    {FONT_NORMAL_GREEN,1072, 0, 6, 0, 11, image_y_offset_default},
    {FONT_NORMAL_BROWN, 1206, 0, 6, 0, 11, image_y_offset_default}
};

static const font_definition DEFINITIONS_EASTERN[] = {
    {FONT_NORMAL_PLAIN,   0, 0, 6, 1, 11, image_y_offset_eastern},
    {FONT_NORMAL_BLACK, 134, 0, 6, 0, 11, image_y_offset_eastern},
    {FONT_NORMAL_WHITE, 268, 0, 6, 0, 11, image_y_offset_eastern},
    {FONT_NORMAL_RED,   402, 0, 6, 0, 11, image_y_offset_eastern},
    {FONT_LARGE_PLAIN,  536, 0, 8, 1, 23, image_y_offset_eastern},
    {FONT_LARGE_BLACK,  670, 0, 8, 0, 23, image_y_offset_eastern},
    {FONT_LARGE_BROWN,  804, 0, 8, 0, 24, image_y_offset_eastern},
    {FONT_SMALL_PLAIN,  938, 0, 4, 1, 9, image_y_offset_eastern},
    {FONT_NORMAL_GREEN,1072, 0, 6, 0, 11, image_y_offset_eastern},
    {FONT_NORMAL_BROWN, 1206, 0, 6, 0, 11, image_y_offset_eastern}
};

static const font_definition DEFINITIONS_CYRILLIC[] = {
    {FONT_NORMAL_PLAIN,   0, 0, 6, 1, 11, image_y_offset_cyrillic_normal_small_plain},
    {FONT_NORMAL_BLACK, 158, 0, 6, 0, 11, image_y_offset_cyrillic_normal_colored},
    {FONT_NORMAL_WHITE, 316, 0, 6, 0, 11, image_y_offset_cyrillic_normal_colored},
    {FONT_NORMAL_RED,   474, 0, 6, 0, 11, image_y_offset_cyrillic_normal_colored},
    {FONT_LARGE_PLAIN,  632, 0, 8, 1, 23, image_y_offset_cyrillic_large_plain},
    {FONT_LARGE_BLACK,  790, 0, 8, 0, 23, image_y_offset_cyrillic_large_black},
    {FONT_LARGE_BROWN,  948, 0, 8, 0, 24, image_y_offset_cyrillic_large_brown},
    {FONT_SMALL_PLAIN, 1106, 0, 4, 1, 9, image_y_offset_cyrillic_normal_small_plain},
    {FONT_NORMAL_GREEN, 1264, 0, 6, 0, 11, image_y_offset_cyrillic_normal_colored},
    {FONT_NORMAL_BROWN, 1422, 0, 6, 0, 11, image_y_offset_cyrillic_normal_brown}
};

static const font_definition DEFINITIONS_TRADITIONAL_CHINESE[] = {
    {FONT_NORMAL_PLAIN, 0, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 1, 15, image_y_offset_chinese},
    {FONT_NORMAL_BLACK, 134, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_WHITE, 268, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_RED, 402, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_LARGE_PLAIN, 536, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS * 2, 8, 1, 23, image_y_offset_chinese},
    {FONT_LARGE_BLACK, 670, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS * 2, 8, 0, 23, image_y_offset_chinese},
    {FONT_LARGE_BROWN, 804, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS * 2, 8, 0, 24, image_y_offset_chinese},
    {FONT_SMALL_PLAIN, 938, 0, 4, 1, 9, image_y_offset_chinese},
    {FONT_NORMAL_GREEN, 1072, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_BROWN, 1206, IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese}
};

static const font_definition DEFINITIONS_SIMPLIFIED_CHINESE[] = {
    {FONT_NORMAL_PLAIN, 0, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 1, 15, image_y_offset_chinese},
    {FONT_NORMAL_BLACK, 134, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_WHITE, 268, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_RED, 402, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_LARGE_PLAIN, 536, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS * 2, 8, 1, 23, image_y_offset_chinese},
    {FONT_LARGE_BLACK, 670, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS * 2, 8, 0, 23, image_y_offset_chinese},
    {FONT_LARGE_BROWN, 804, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS * 2, 8, 0, 24, image_y_offset_chinese},
    {FONT_SMALL_PLAIN, 938, 0, 4, 1, 9, image_y_offset_none},
    {FONT_NORMAL_GREEN, 1072, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese},
    {FONT_NORMAL_BROWN, 1206, IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS, 6, 0, 15, image_y_offset_chinese}
};

static const font_definition DEFINITIONS_KOREAN[] = {
    {FONT_NORMAL_PLAIN, 0, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS * 1, 6, 1, 15, image_y_offset_korean},
    {FONT_NORMAL_BLACK, 134, 0, 6, 0, 11, image_y_offset_korean},
    {FONT_NORMAL_WHITE, 268, 0, 6, 0, 11, image_y_offset_korean},
    {FONT_NORMAL_RED, 402, 0, 6, 0, 11, image_y_offset_korean},
    {FONT_LARGE_PLAIN, 536, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS * 2, 8, 1, 23, image_y_offset_korean},
    {FONT_LARGE_BLACK, 670, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS * 2, 8, 0, 23, image_y_offset_korean},
    {FONT_LARGE_BROWN, 804, IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS * 2, 8, 0, 24, image_y_offset_korean},
    {FONT_SMALL_PLAIN, 938, 0, 4, 1, 9, image_y_offset_korean},
    {FONT_NORMAL_GREEN, 1072, 0, 6, 0, 11, image_y_offset_korean},
    {FONT_NORMAL_BROWN, 1206, 0, 6, 0, 11, image_y_offset_korean}
};

enum {
    MULTIBYTE_NONE = 0,
    MULTIBYTE_TRADITIONAL_CHINESE = 1,
    MULTIBYTE_SIMPLIFIED_CHINESE = 2,
    MULTIBYTE_KOREAN = 3,
};

static struct {
    const int *font_mapping;
    const font_definition *font_definitions;
    int multibyte;
} data;

static int image_y_offset_none(uint8_t c, int image_height, int line_height)
{
    int offset = image_height - line_height;
    if (offset < 0 || c < 0x80) {
        offset = 0;
    }
    return offset;
}

static int image_y_offset_default(uint8_t c, int image_height, int line_height)
{
    int offset = image_height - line_height;
    if (offset < 0) {
        offset = 0;
    }
    if (c < 0x80 || c == 0xE7) {
        offset = 0;
    }
    return offset;
}

static int image_y_offset_eastern(uint8_t c, int image_height, int line_height)
{
    int offset = image_height - line_height;
    if (offset < 0) {
        offset = 0;
    }
    if (c < 0x80 || c == 0xEA || c == 0xB9 || c == 0xA5 || c == 0xCA) {
        offset = 0;
    }
    return offset;
}

static int image_y_offset_cyrillic_normal_small_plain(uint8_t c, int image_height, int line_height)
{
    switch (c) {
        case 36:
            return 1;
        case 201:
            return 3;
        case 225:
            return 1;
        default:
            return 0;
    }
}

static int image_y_offset_cyrillic_normal_colored(uint8_t c, int image_height, int line_height)
{
    return c == 201 ? 3 : 0;
}

static int image_y_offset_cyrillic_large_plain(uint8_t c, int image_height, int line_height)
{
    switch (c) {
        case 36:
            return 2;
        case 201:
            return 7;
        case 35:
        case 42:
        case 47:
        case 64:
        case 92:
        case 98:
        case 100:
        case 102:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 124:
        case 225:
        case 244:
            return 1;
        default:
            return 0;
    }
}

static int image_y_offset_cyrillic_large_black(uint8_t c, int image_height, int line_height)
{
    switch (c) {
        case 36:
            return 2;
        case 201:
            return 7;
        case 35:
        case 40:
        case 41:
        case 42:
        case 47:
        case 64:
        case 92:
        case 98:
        case 100:
        case 102:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 123:
        case 124:
        case 125:
        case 225:
        case 244:
            return 1;
        default:
            return 0;
    }
}

static int image_y_offset_cyrillic_large_brown(uint8_t c, int image_height, int line_height)
{
    switch (c) {
        case 36:
            return 2;
        case 201:
            return 7;
        case 40:
        case 41:
        case 42:
        case 47:
        case 64:
        case 92:
        case 96:
        case 98:
        case 100:
        case 102:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 123:
        case 124:
        case 125:
        case 225:
        case 244:
            return 1;
        default:
            return 0;
    }
}

static int image_y_offset_cyrillic_normal_brown(uint8_t c, int image_height, int line_height)
{
    switch (c) {
        case 36:
        case 40:
        case 41:
        case 42:
        case 47:
        case 64:
        case 92:
        case 98:
        case 100:
        case 102:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 123:
        case 124:
        case 125:
        case 225:
        case 244:
            return 1;
        case 201:
            return 4;
        default:
            return 0;
    }
}

static int image_y_offset_chinese(uint8_t c, int image_height, int line_height)
{
    if (line_height == 15) {
        if (c < 0x80) {
            return 0;
        } else {
            return 2;
        }
    }
    if (c < 0x80) {
        return 0;
    }
    return image_height - line_height;
}

static int image_y_offset_korean(uint8_t c, int image_height, int line_height)
{
    if (line_height == 15) {
        if (c < 0x80) {
            return -1;
        } else {
            return 2;
        }
    }
    if (c < 0x80) {
        return 0;
    }
    if (line_height == 11) {
        if (image_height == 12) {
            return 0;
        } else if (image_height == 15) {
            return 3;
        }
    }
    return image_height - line_height;
}

void font_set_encoding(encoding_type encoding)
{
    data.multibyte = MULTIBYTE_NONE;
    if (encoding == ENCODING_EASTERN_EUROPE) {
        data.font_mapping = CHAR_TO_FONT_IMAGE_EASTERN;
        data.font_definitions = DEFINITIONS_EASTERN;
    } else if (encoding == ENCODING_CYRILLIC) {
        data.font_mapping = CHAR_TO_FONT_IMAGE_CYRILLIC;
        data.font_definitions = DEFINITIONS_CYRILLIC;
    } else if (encoding == ENCODING_TRADITIONAL_CHINESE) {
        data.font_mapping = CHAR_TO_FONT_IMAGE_DEFAULT;
        data.font_definitions = DEFINITIONS_TRADITIONAL_CHINESE;
        data.multibyte = MULTIBYTE_TRADITIONAL_CHINESE;
    } else if (encoding == ENCODING_SIMPLIFIED_CHINESE) {
        data.font_mapping = CHAR_TO_FONT_IMAGE_DEFAULT;
        data.font_definitions = DEFINITIONS_SIMPLIFIED_CHINESE;
        data.multibyte = MULTIBYTE_SIMPLIFIED_CHINESE;
    } else if (encoding == ENCODING_KOREAN) {
        data.font_mapping = CHAR_TO_FONT_IMAGE_DEFAULT;
        data.font_definitions = DEFINITIONS_KOREAN;
        data.multibyte = MULTIBYTE_KOREAN;
    } else {
        data.font_mapping = CHAR_TO_FONT_IMAGE_DEFAULT;
        data.font_definitions = DEFINITIONS_DEFAULT;
    }
}

const font_definition *font_definition_for(font_t font)
{
    return &data.font_definitions[font];
}

int font_can_display(const uint8_t *character)
{
    int dummy;
    return font_letter_id(&data.font_definitions[FONT_NORMAL_BLACK], character, &dummy) >= 0;
}

int font_letter_id(const font_definition *def, const uint8_t *str, int *num_bytes)
{
    if (data.multibyte != MULTIBYTE_NONE && *str >= 0x80) {
        *num_bytes = 2;
        if (data.multibyte == MULTIBYTE_TRADITIONAL_CHINESE) {
            int char_id = (str[0] & 0x7f) | ((str[1] & 0x7f) << 7);
            if (char_id >= IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS) {
                // lookup in table
                int big5_encoded = str[0] << 8 | str[1];
                char_id = encoding_trad_chinese_big5_to_image_id(big5_encoded);
                if (char_id < 0 || char_id >= IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS) {
                    return -1;
                }
            }
            return IMAGE_FONT_MULTIBYTE_OFFSET + def->multibyte_image_offset + char_id;
        } else if (data.multibyte == MULTIBYTE_SIMPLIFIED_CHINESE) {
            int char_id = (str[0] & 0x7f) | ((str[1] & 0x7f) << 7);
            if (char_id >= IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS) {
                return -1;
            }
            return IMAGE_FONT_MULTIBYTE_OFFSET + def->multibyte_image_offset + char_id;
        } else if (data.multibyte == MULTIBYTE_KOREAN) {
            int b0 = str[0] - 0xb0;
            int b1 = str[1] - 0xa1;
            int char_id = b0 * 94 + b1;
            if (b0 < 0 || b1 < 0 || char_id < 0 || char_id >= IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS) {
                return -1;
            }
            return IMAGE_FONT_MULTIBYTE_OFFSET + def->multibyte_image_offset + char_id;
        } else {
            return -1;
        }
    } else {
        *num_bytes = 1;
        if (!data.font_mapping[*str]) {
            return -1;
        }
        return data.font_mapping[*str] + def->image_offset - 1;
    }
}
