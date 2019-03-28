#include "text.h"

#include "core/string.h"
#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/image.h"

#include <string.h>

static uint8_t tmp_line[200];

static struct {
    int capture;
    int seen;
    int position;
    int cursor_position;
    int width;
    int visible;
    time_millis updated;
    int x_offset;
    int y_offset;
} input_cursor;

static struct {
    const char * string;
    int width[FONT_TYPES_MAX];
} ellipsis = { "..." };

static int get_ellipsis_width(font_t font)
{
    if (!ellipsis.width[font]) {
        ellipsis.width[font] = text_get_width(string_from_ascii(ellipsis.string), font);
    }
    return ellipsis.width[font];
}

void text_capture_cursor(int cursor_position)
{
    input_cursor.capture = 1;
    input_cursor.seen = 0;
    input_cursor.position = 0;
    input_cursor.width = 0;
    input_cursor.cursor_position = cursor_position;
}

void text_draw_cursor(int x_offset, int y_offset, int is_insert)
{
    input_cursor.capture = 0;
    time_millis curr = time_get_millis();
    time_millis diff = curr - input_cursor.updated;
    if (!input_cursor.visible && diff >= 200) {
        input_cursor.visible = 1;
        input_cursor.updated = curr;
    } else if (input_cursor.visible && diff >= 400) {
        input_cursor.visible = 0;
        input_cursor.updated = curr;
    }
    if (input_cursor.visible) {
        if (is_insert) {
            graphics_draw_horizontal_line(
                x_offset + input_cursor.x_offset - 3, x_offset + input_cursor.x_offset + 1,
                y_offset + input_cursor.y_offset - 3, COLOR_WHITE);
            graphics_draw_vertical_line(
                x_offset + input_cursor.x_offset - 1,  y_offset + input_cursor.y_offset - 3,
                y_offset + input_cursor.y_offset + 13, COLOR_WHITE);
            graphics_draw_horizontal_line(
                x_offset + input_cursor.x_offset - 3,  x_offset + input_cursor.x_offset + 1,
                y_offset + input_cursor.y_offset + 14, COLOR_WHITE);
        } else {
            graphics_fill_rect(
                x_offset + input_cursor.x_offset, y_offset + input_cursor.y_offset + 14,
                input_cursor.width, 2, COLOR_WHITE);
        }
    }
}

int text_get_width(const uint8_t *str, font_t font)
{
    const font_definition *def = font_definition_for(font);
    int maxlen = 10000;
    int width = 0;
    int image_base = image_group(GROUP_FONT);
    while (*str && maxlen > 0) {
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int image_offset = font_image_for(*str);
            if (image_offset) {
                int image_id = image_base + def->image_offset + image_offset - 1;
                width += def->letter_spacing + image_get(image_id)->width;
            }
        }
        str++;
        maxlen--;
    }
    return width;
}

void text_ellipsize(char *str, font_t font, int requested_width)
{
    char *orig_str = str;
    const font_definition *def = font_definition_for(font);
    int ellipsis_width = get_ellipsis_width(font);
    unsigned int maxlen = 10000;
    int width = 0;
    int length_with_ellipsis = 0;
    int image_base = image_group(GROUP_FONT);
    while (*str && maxlen > 0) {
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int image_offset = font_image_for(*str);
            if (image_offset) {
                int image_id = image_base + def->image_offset + image_offset - 1;
                width += def->letter_spacing + image_get(image_id)->width;
            }
        }
        if (ellipsis_width + width <= requested_width) {
            length_with_ellipsis++;
        }
        if (width > requested_width) {
            break;
        }
        str++;
        maxlen--;
    }
    if (10000 - maxlen < strlen(orig_str)) {
        strcpy(orig_str + length_with_ellipsis, ellipsis.string);
    }
}

static int get_character_width(uint8_t c, const font_definition *def)
{
    int image_offset = font_image_for(c);
    if (!image_offset) {
        return 0;
    }
    int image_id = image_group(GROUP_FONT) + def->image_offset + image_offset - 1;
    return 1 + image_get(image_id)->width;
}

static int get_word_width(const uint8_t *str, font_t font, int *out_num_chars)
{
    const font_definition *def = font_definition_for(font);
    int width = 0;
    int guard = 0;
    int word_char_seen = 0;
    int num_chars = 0;
    for (uint8_t c = *str; c; c = *++str) {
        if (++guard >= 200) {
            break;
        }
        if (c == ' ') {
            if (word_char_seen) {
                break;
            }
            width += 4;
        } else if (c == '$') {
            if (word_char_seen) {
                break;
            }
        } else if (c > ' ') {
            // normal char
            width += get_character_width(c, def);
            word_char_seen = 1;
        }
        num_chars++;
    }
    *out_num_chars = num_chars;
    return width;
}

void text_draw_centered(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color)
{
    int offset = (box_width - text_get_width(str, font)) / 2;
    if (offset < 0) {
        offset = 0;
    }
    text_draw(str, offset + x, y, font, color);
}

static int draw_character(const font_definition *def, unsigned int c, int x, int y, color_t color)
{
    int image_offset = font_image_for(c);
    int image_id = image_group(GROUP_FONT) + def->image_offset + image_offset - 1;
    const image *img = image_get(image_id);
    int height = font_image_height_offset(c, img->height, def->line_height);
    image_draw_letter(image_id, x, y - height, color);
    return img->width;
}

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color)
{
    const font_definition *def = font_definition_for(font);

    int current_x = x;
    while (*str) {
        uint8_t c = *str;

        if (c == '_') {
            c = ' ';
        }

        if (c >= ' ') {
            int graphic = font_image_for(c);
            int width;
            if (graphic == 0) {
                width = def->space_width_draw;
            } else {
                width = def->letter_spacing_draw + draw_character(def, c, current_x, y, color);
            }
            if (input_cursor.capture && input_cursor.position == input_cursor.cursor_position) {
                if (!input_cursor.seen) {
                    input_cursor.width = width;
                    input_cursor.x_offset = current_x - x;
                    input_cursor.seen = 1;
                }
            }
            current_x += width;
        }

        str++;
        input_cursor.position++;
    }
    if (input_cursor.capture && !input_cursor.seen) {
        input_cursor.width = 4;
        input_cursor.x_offset = current_x - x;
        input_cursor.seen = 1;
    }
    current_x += def->space_width_draw;
    return current_x - x;
}

static void number_to_string(uint8_t *str, int value, char prefix, const char *postfix)
{
    int offset = 0;
    if (prefix) {
        str[offset++] = prefix;
    }
    offset += string_from_int(&str[offset], value, 0);
    while (*postfix) {
        str[offset++] = *postfix;
        postfix++;
    }
    str[offset] = 0;
}

int text_draw_number(int value, char prefix, const char *postfix, int x_offset, int y_offset, font_t font)
{
    uint8_t str[100];
    number_to_string(str, value, prefix, postfix);
    return text_draw(str, x_offset, y_offset, font, 0);
}

int text_draw_number_colored(int value, char prefix, const char *postfix, int x_offset, int y_offset, font_t font, color_t color)
{
    uint8_t str[100];
    number_to_string(str, value, prefix, postfix);
    return text_draw(str, x_offset, y_offset, font, color);
}

int text_draw_money(int value, int x_offset, int y_offset, font_t font)
{
    uint8_t str[100];
    number_to_string(str, value, '@', " Dn");
    return text_draw(str, x_offset, y_offset, font, 0);
}

int text_draw_percentage(int value, int x_offset, int y_offset, font_t font)
{
    uint8_t str[100];
    number_to_string(str, value, '@', "%");
    return text_draw(str, x_offset, y_offset, font, 0);
}

void text_draw_number_centered(int value, int x_offset, int y_offset, int box_width, font_t font)
{
    uint8_t str[100];
    number_to_string(str, value, '@', " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void text_draw_number_centered_colored(int value, int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    uint8_t str[100];
    number_to_string(str, value, '@', " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, color);
}

int text_draw_multiline(const uint8_t *str, int x_offset, int y_offset, int box_width, font_t font)
{
    int line_height;
    switch (font) {
        case FONT_LARGE_PLAIN:
        case FONT_LARGE_BLACK:
            line_height = 23;
            break;
        case FONT_LARGE_BROWN:
            line_height = 24;
            break;
        case FONT_SMALL_PLAIN:
            line_height = 9;
            break;
        case FONT_NORMAL_PLAIN:
            line_height = 11;
            break;
        default:
            line_height = 11;
            break;
    }

    int has_more_characters = 1;
    int guard = 0;
    int y = y_offset;
    while (has_more_characters) {
        if (++guard >= 100) {
            break;
        }
        // clear line
        for (int i = 0; i < 200; i++) {
            tmp_line[i] = 0;
        }
        int current_width = 0;
        int line_index = 0;
        while (has_more_characters && current_width < box_width) {
            int word_num_chars;
            int word_width = get_word_width(str, font, &word_num_chars);
            current_width += word_width;
            if (current_width >= box_width) {
                if (current_width == 0) {
                    has_more_characters = 0;
                }
            } else {
                for (int i = 0; i < word_num_chars; i++) {
                    tmp_line[line_index++] = *str++;
                }
                if (!*str) {
                    has_more_characters = 0;
                }
            }
        }
        text_draw(tmp_line, x_offset, y, font, 0);
        y += line_height + 5;
    }
    return y - y_offset;
}
