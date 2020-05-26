#include "text.h"

#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/image.h"

#include <string.h>

#define ELLIPSIS_LENGTH 4
#define NUMBER_BUFFER_LENGTH 100

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
    int text_offset_start;
    int text_offset_end;
} input_cursor;

static struct {
    const uint8_t string[ELLIPSIS_LENGTH];
    int width[FONT_TYPES_MAX];
} ellipsis = { {'.', '.', '.', 0} };

static int get_ellipsis_width(font_t font)
{
    if (!ellipsis.width[font]) {
        ellipsis.width[font] = text_get_width(ellipsis.string, font);
    }
    return ellipsis.width[font];
}

void text_capture_cursor(int cursor_position, int offset_start, int offset_end)
{
    input_cursor.capture = 1;
    input_cursor.seen = 0;
    input_cursor.position = 0;
    input_cursor.width = 0;
    input_cursor.cursor_position = cursor_position;
    input_cursor.text_offset_start = offset_start;
    input_cursor.text_offset_end = offset_end;
}

void text_draw_cursor(int x_offset, int y_offset, int is_insert)
{
    if (!input_cursor.capture) {
        return;
    }
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
    while (*str && maxlen > 0) {
        int num_bytes = 1;
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id >= 0) {
                width += def->letter_spacing + image_letter(letter_id)->width;
            }
        }
        str += num_bytes;
        maxlen -= num_bytes;
    }
    return width;
}

unsigned int text_get_max_length_for_width(const uint8_t *str, int length, font_t font, unsigned int requested_width, int invert)
{
    const font_definition *def = font_definition_for(font);
    length = (!length) ? string_length(str) : length;
    unsigned int maxlen = length;
    unsigned int width = 0;
    int step = 1;
    if (invert) {
        str += length - 1;
        step = -1;
    }
    while (maxlen) {
        // TODO FIX THIS: MULTIBYTE CANNOT BE TRAVERSED BACKWARDS
        int num_bytes = 1;
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id >= 0) {
                width += def->letter_spacing + image_letter(letter_id)->width;
            }
        }
        if (width > requested_width) {
            break;
        }
        str += step * num_bytes;
        maxlen -= num_bytes;
    }
    return length - maxlen;
}

void text_ellipsize(uint8_t *str, font_t font, int requested_width)
{
    uint8_t *orig_str = str;
    const font_definition *def = font_definition_for(font);
    int ellipsis_width = get_ellipsis_width(font);
    int maxlen = 10000;
    int width = 0;
    int length_with_ellipsis = 0;
    while (*str && maxlen > 0) {
        int num_bytes = 1;
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id >= 0) {
                width += def->letter_spacing + image_letter(letter_id)->width;
            }
        }
        if (ellipsis_width + width <= requested_width) {
            length_with_ellipsis++;
        }
        if (width > requested_width) {
            break;
        }
        str += num_bytes;
        maxlen -= num_bytes;
    }
    if (10000 - maxlen < string_length(orig_str)) {
        string_copy(ellipsis.string, orig_str + length_with_ellipsis, ELLIPSIS_LENGTH);
    }
}

static int get_word_width(const uint8_t *str, font_t font, int *out_num_chars)
{
    const font_definition *def = font_definition_for(font);
    int width = 0;
    int guard = 0;
    int word_char_seen = 0;
    int num_chars = 0;
    while (*str && ++guard < 200) {
        int num_bytes = 1;
        if (*str == ' ' || *str == '\n') {
            if (word_char_seen) {
                break;
            }
            width += def->space_width;
        } else if (*str == '$') {
            if (word_char_seen) {
                break;
            }
        } else if (*str > ' ') {
            // normal char
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id >= 0) {
                width += image_letter(letter_id)->width + def->letter_spacing;
            }
            word_char_seen = 1;
            if (num_bytes > 1) {
                num_chars += num_bytes;
                break;
            }
        }
        str += num_bytes;
        num_chars += num_bytes;
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

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color)
{
    const font_definition *def = font_definition_for(font);

    int length = string_length(str);
    if (input_cursor.capture) {
        str += input_cursor.text_offset_start;
        length = input_cursor.text_offset_end - input_cursor.text_offset_start;
    }

    int current_x = x;
    while (length > 0) {
        int num_bytes = 1;

        if (*str >= ' ') {
            int letter_id = font_letter_id(def, str, &num_bytes);
            int width;
            if (*str == ' ' || *str == '_' || letter_id < 0) {
                width = def->space_width;
            } else {
                const image *img = image_letter(letter_id);
                int height = def->image_y_offset(*str, img->height, def->line_height);
                image_draw_letter(def->font, letter_id, current_x, y - height, color);
                width = def->letter_spacing + img->width;
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

        str += num_bytes;
        length -= num_bytes;
        input_cursor.position++;
    }
    if (input_cursor.capture && !input_cursor.seen) {
        input_cursor.width = 4;
        input_cursor.x_offset = current_x - x;
        input_cursor.seen = 1;
    }
    current_x += def->space_width;
    return current_x - x;
}

static int number_to_string(uint8_t *str, int value, char prefix, const char *postfix)
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
    return offset;
}

int text_draw_number(int value, char prefix, const char *postfix, int x_offset, int y_offset, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, prefix, postfix);
    return text_draw(str, x_offset, y_offset, font, 0);
}

int text_draw_number_colored(int value, char prefix, const char *postfix, int x_offset, int y_offset, font_t font, color_t color)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, prefix, postfix);
    return text_draw(str, x_offset, y_offset, font, color);
}

int text_draw_money(int value, int x_offset, int y_offset, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    int money_len = number_to_string(str, value, '@', " ");
    const uint8_t *postfix = lang_get_string(6, 0);
    if (postfix) {
        string_copy(postfix, str + money_len, NUMBER_BUFFER_LENGTH - money_len - 1);
    }
    return text_draw(str, x_offset, y_offset, font, 0);
}

int text_draw_percentage(int value, int x_offset, int y_offset, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, '@', "%");
    return text_draw(str, x_offset, y_offset, font, 0);
}

void text_draw_number_centered(int value, int x_offset, int y_offset, int box_width, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, '@', " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void text_draw_number_centered_prefix(int value, char prefix, int x_offset, int y_offset, int box_width, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, prefix, " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void text_draw_number_centered_colored(int value, int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, '@', " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, color);
}

int text_draw_multiline(const uint8_t *str, int x_offset, int y_offset, int box_width, font_t font, uint32_t color)
{
    int line_height = font_definition_for(font)->line_height;
    if (line_height < 11) {
        line_height = 11;
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
                    if (line_index == 0 && *str <= ' ') {
                        str++; // skip whitespace at start of line
                    } else {
                        tmp_line[line_index++] = *str++;
                    }
                }
                if (!*str) {
                    has_more_characters = 0;
                } else if (*str == '\n') {
                    str++;
                    break;
                }
            }
        }
        text_draw(tmp_line, x_offset, y, font, color);
        y += line_height + 5;
    }
    return y - y_offset;
}

int text_measure_multiline(const uint8_t *str, int box_width, font_t font)
{
    int has_more_characters = 1;
    int guard = 0;
    int num_lines = 0;
    while (has_more_characters) {
        if (++guard >= 100) {
            break;
        }
        int current_width = 0;
        while (has_more_characters && current_width < box_width) {
            int word_num_chars;
            int word_width = get_word_width(str, font, &word_num_chars);
            current_width += word_width;
            if (current_width >= box_width) {
                if (current_width == 0) {
                    has_more_characters = 0;
                }
            } else {
                str += word_num_chars;
                if (!*str) {
                    has_more_characters = 0;
                } else if (*str == '\n') {
                    str++;
                    break;
                }
            }
        }
        num_lines += 1;
    }
    return num_lines;
}
