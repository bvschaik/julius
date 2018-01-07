#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include <stdint.h>

typedef enum {
    FONT_NORMAL_PLAIN,
    FONT_NORMAL_BLACK,
    FONT_NORMAL_WHITE,
    FONT_NORMAL_RED,
    FONT_LARGE_PLAIN,
    FONT_LARGE_BLACK,
    FONT_LARGE_BROWN,
    FONT_SMALL_PLAIN,
    FONT_NORMAL_GREEN,
    FONT_SMALL_BLACK
} font_t;

typedef struct {
    font_t font;
    int image_offset;
    int space_width;
    int space_width_draw;
    int letter_spacing;
    int letter_spacing_draw;
    int line_height;
} font_definition;

const font_definition *font_definition_for(font_t font);

int font_image_for(uint8_t c);

#endif // GRAPHICS_FONT_H
