#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include "core/encoding.h"

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
    FONT_SMALL_BLACK,
    FONT_TYPES_MAX
} font_t;

typedef struct {
    font_t font;
    int image_offset;
    int multibyte_image_offset;
    int space_width;
    int letter_spacing;
    int line_height;

    /**
    * Returns the height offset for the specified character
    * @param c Character
    * @param image_height Height of the letter image
    * @param line_height Line height for the font
    * @return Offset to subtract from y coordinate
    */
    int (*image_y_offset)(uint8_t c, int image_height, int line_height);
} font_definition;

/**
 * Sets the encoding for font drawing functions
 * @param encoding Encoding to use
 */
void font_set_encoding(encoding_type encoding);

/**
 * Gets the font definition for the specified font
 * @param font Font
 * @return Font definition
 */
const font_definition *font_definition_for(font_t font);

/**
 * Checks whether the font has a glyph for the passed character
 * @param character Character to check
 * @return Boolean true if this character can be drawn on the screen, false otherwise
 */
int font_can_display(const uint8_t *character);

/**
 * Gets the letter ID for the specified character and font
 * @param def Font definition
 * @param str Character string
 * @param num_bytes Out: number of bytes consumed by letter
 * @return Letter ID to feed into image_letter(), or -1 if c is no letter
 */
int font_letter_id(const font_definition *def, const uint8_t *str, int *num_bytes);

#endif // GRAPHICS_FONT_H
