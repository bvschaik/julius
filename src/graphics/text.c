#include "text.h"

#include "core/config.h"
#include "core/lang.h"
#include "core/locale.h"
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
            graphics_draw_line(
                x_offset + input_cursor.x_offset - 3, x_offset + input_cursor.x_offset + 1,
                y_offset + input_cursor.y_offset - 3, y_offset + input_cursor.y_offset - 3, COLOR_WHITE);
            graphics_draw_line(
                x_offset + input_cursor.x_offset - 1, x_offset + input_cursor.x_offset - 1, 
                y_offset + input_cursor.y_offset - 3, y_offset + input_cursor.y_offset + 13, COLOR_WHITE);
            graphics_draw_line(
                x_offset + input_cursor.x_offset - 3, x_offset + input_cursor.x_offset + 1,
                y_offset + input_cursor.y_offset + 14, y_offset + input_cursor.y_offset + 14, COLOR_WHITE);
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
                width += def->letter_spacing + image_letter(letter_id)->original.width;
            }
        }
        str += num_bytes;
        maxlen -= num_bytes;
    }
    return width;
}

int text_get_number_width(int value, char prefix, const char *postfix, font_t font)
{
    const font_definition *def = font_definition_for(font);

    int width = 0;

    if (prefix) {
        uint8_t prefix_str[2] = { prefix, 0 };
        width += text_get_width(prefix_str, font);
    }

    uint8_t buffer[NUMBER_BUFFER_LENGTH];
    int length = string_from_int(buffer, value, 0);
    uint8_t *str = buffer;

    int separator_pixels = config_get(CONFIG_UI_DIGIT_SEPARATOR) * 3;

    while (length > 0) {
        int num_bytes = 1;

        if (*str >= ' ') {
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (*str == ' ' || *str == '_' || letter_id < 0) {
                width += def->space_width;
            } else {
                const image *img = image_letter(letter_id);
                width += def->letter_spacing + img->original.width;
            }
            if (length == 4 || length == 7) {
                width += separator_pixels;
            }
        }

        str += num_bytes;
        length -= num_bytes;
    }

    if (postfix && *postfix) {
        width += text_get_width(string_from_ascii(postfix), font);
    }
    return width;
}

static int get_letter_width(const uint8_t *str, const font_definition *def, int *num_bytes)
{
    *num_bytes = 1;
    if (*str == ' ') {
        return def->space_width;
    }
    int letter_id = font_letter_id(def, str, num_bytes);
    if (letter_id >= 0) {
        return def->letter_spacing + image_letter(letter_id)->original.width;
    } else {
        return 0;
    }
}

unsigned int text_get_max_length_for_width(
    const uint8_t *str, int length, font_t font, unsigned int requested_width, int invert)
{
    const font_definition *def = font_definition_for(font);
    if (!length) {
        length = string_length(str);
    }
    if (invert) {
        unsigned int maxlen = length;
        unsigned int width = 0;
        const uint8_t *s = str;
        while (maxlen) {
            int num_bytes;
            width += get_letter_width(s, def, &num_bytes);
            s += num_bytes;
            maxlen -= num_bytes;
        }

        maxlen = length;
        while (maxlen && width > requested_width) {
            int num_bytes;
            width -= get_letter_width(str, def, &num_bytes);
            str += num_bytes;
            maxlen -= num_bytes;
        }
        return maxlen;
    } else {
        unsigned int maxlen = length;
        unsigned int width = 0;
        while (maxlen) {
            int num_bytes;
            width += get_letter_width(str, def, &num_bytes);
            if (width > requested_width) {
                break;
            }
            str += num_bytes;
            maxlen -= num_bytes;
        }
        return length - maxlen;
    }
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
                width += def->letter_spacing + image_letter(letter_id)->original.width;
            }
        }
        if (ellipsis_width + width <= requested_width) {
            length_with_ellipsis += num_bytes;
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
                width += image_letter(letter_id)->original.width + def->letter_spacing;
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

void text_draw_centered_with_linebreaks(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color)
{
    int count = 0;
    char *split;

    char oldstr[512];
    strcpy(oldstr, (char *) str);

    split = strtok(oldstr, "\n");
    while (split != NULL) {
        text_draw_centered((uint8_t *) split, x, y + (20 * count), box_width, font, color);
        count++;
        split = strtok(NULL, "\n");
    }
}

void text_draw_centered(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color)
{
    int offset = (box_width - text_get_width(str, font)) / 2;
    if (offset < 0) {
        offset = 0;
    }
    text_draw(str, offset + x, y, font, color);
}

int text_draw_ellipsized(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color)
{
    static uint8_t buffer[1000];
    string_copy(str, buffer, 1000);
    text_ellipsize(buffer, font, box_width);
    return text_draw(buffer, x, y, font, color);
}

int text_draw_scaled(const uint8_t *str, int x, int y, font_t font, color_t color, float scale)
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
                int height = def->image_y_offset(*str, img->height + img->y_offset, def->line_height);
                image_draw_letter(def->font, letter_id, current_x, y - height, color, scale);
                width = def->letter_spacing + img->original.width;
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
        input_cursor.position += num_bytes;
    }
    if (input_cursor.capture && !input_cursor.seen) {
        input_cursor.width = 4;
        input_cursor.x_offset = current_x - x;
        input_cursor.seen = 1;
    }
    current_x += def->space_width;
    return current_x - x;
}

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color)
{
    return text_draw_scaled(str, x, y, font, color, SCALE_NONE);
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

int text_draw_number_scaled(int value, char prefix, const char *postfix,
    int x, int y, font_t font, color_t color, float scale)
{
    const font_definition *def = font_definition_for(font);
    int current_x = x;

    if (prefix) {
        uint8_t prefix_str[2] = { prefix, 0 };
        current_x += text_draw_scaled(prefix_str, current_x, y, font, color, scale) - def->space_width;
    }

    uint8_t buffer[NUMBER_BUFFER_LENGTH];
    int length = string_from_int(buffer, value, 0);
    uint8_t *str = buffer;

    int separator_pixels = config_get(CONFIG_UI_DIGIT_SEPARATOR) * 3;

    while (length > 0) {
        int num_bytes = 1;

        if (*str >= ' ') {
            int letter_id = font_letter_id(def, str, &num_bytes);
            int width;
            if (*str == ' ' || *str == '_' || letter_id < 0) {
                width = def->space_width;
            } else {
                const image *img = image_letter(letter_id);
                int height = def->image_y_offset(*str, img->height + img->y_offset, def->line_height);
                image_draw_letter(def->font, letter_id, current_x, y - height, color, scale);
                width = def->letter_spacing + img->width;
            }

            current_x += width + ((length == 4 || length == 7) ? separator_pixels : 0);
        }

        str += num_bytes;
        length -= num_bytes;
    }

    if (postfix && *postfix) {
        current_x += text_draw_scaled(string_from_ascii(postfix), current_x, y, font, color, scale);
    } else {
        current_x += def->space_width;
    }

    return current_x - x;
}

int text_draw_number(int value, char prefix, const char *postfix, int x, int y, font_t font, color_t color)
{
    return text_draw_number_scaled(value, prefix, postfix, x, y, font, color, SCALE_NONE);
}

void text_draw_number_finances(int value, int x, int y, font_t font, color_t color)
{
    const font_definition *def = font_definition_for(font);
    int number_width = 10;
    int current_x = x - number_width;

    uint8_t buffer[NUMBER_BUFFER_LENGTH];
    int length = string_from_int(buffer, value, 0);
    uint8_t *str = &buffer[length - 1];
    int inverted_length = 0;

    int separator_pixels = config_get(CONFIG_UI_DIGIT_SEPARATOR) * 4;

    while (length > 0) {
        int num_bytes = 1;

        if (*str >= ' ') {
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (*str != ' ' && *str != '_' && letter_id >= 0) {
                if (*str != '-') {
                    current_x -= !(inverted_length % 3) ? separator_pixels : 0;
                }
                const image *img = image_letter(letter_id);
                int height = def->image_y_offset(*str, img->height + img->y_offset, def->line_height);
                image_draw_letter(def->font, letter_id, current_x + (number_width - img->width) / 2, y - height, color, SCALE_NONE);
            }
            current_x -= number_width;
        }

        str--;
        length--;
        inverted_length++;
    }
}

int text_draw_money(int value, int x_offset, int y_offset, font_t font)
{
    const uint8_t *postfix;
    if (locale_translate_money_dn()) {
        postfix = lang_get_string(6, 0);
    } else {
        postfix = string_from_ascii("Dn");
    }
    int offset = text_draw_number(value, '@', 0, x_offset, y_offset, font, 0);
    offset += text_draw(postfix, x_offset + offset, y_offset, font, 0);
    return offset;
}

void text_draw_with_money(const uint8_t *text, int value, const char *prefix, const char *postfix,
    int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    uint8_t *offset = string_copy(text, str, NUMBER_BUFFER_LENGTH);
    if (prefix && *prefix) {
        offset = string_copy(string_from_ascii(prefix), offset, NUMBER_BUFFER_LENGTH - (int) (offset - str) - 1);
    }
    offset += number_to_string(offset, value, 0, " ");
    const uint8_t *money_postfix;
    if (locale_translate_money_dn()) {
        money_postfix = lang_get_string(6, 0);
    } else {
        money_postfix = string_from_ascii("Dn");
    }
    offset = string_copy(money_postfix, offset, NUMBER_BUFFER_LENGTH - (int) (offset - str) - 1);
    if (postfix && *postfix) {
        string_copy(string_from_ascii(postfix), offset, NUMBER_BUFFER_LENGTH - (int) (offset - str) - 1);
    }
    if (box_width > 0) {
        text_draw_centered(str, x_offset, y_offset, box_width, font, color);
    } else {
        text_draw(str, x_offset, y_offset, font, color);
    }
}

int text_draw_percentage(int value, int x_offset, int y_offset, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, '@', "%");
    return text_draw(str, x_offset, y_offset, font, 0);
}

int text_draw_label_and_number(const uint8_t *label, int value, const char *postfix, int x_offset, int y_offset, font_t font, color_t color)
{
    uint8_t str[2 * NUMBER_BUFFER_LENGTH];
    uint8_t *pos = label ? string_copy(label, str, NUMBER_BUFFER_LENGTH) : str;
    number_to_string(pos, value, '@', postfix);
    return text_draw(str, x_offset, y_offset, font, color);
}

void text_draw_label_and_number_centered(const uint8_t *label, int value, const char *postfix, int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    uint8_t str[2 * NUMBER_BUFFER_LENGTH];
    uint8_t *pos = label ? string_copy(label, str, NUMBER_BUFFER_LENGTH) : str;
    number_to_string(pos, value, '@', postfix);
    text_draw_centered(str, x_offset, y_offset, box_width, font, color);
}

void text_draw_number_centered(int value, int x_offset, int y_offset, int box_width, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, '@', " ");
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void text_draw_number_centered_with_separator(int value, int x_offset, int y_offset, int box_width, font_t font)
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

void text_draw_number_centered_postfix(int value, const char *postfix, int x_offset, int y_offset, int box_width, font_t font)
{
    uint8_t str[NUMBER_BUFFER_LENGTH];
    number_to_string(str, value, 0, postfix);
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void text_draw_number_centered_colored(
    int value, int x_offset, int y_offset, int box_width, font_t font, color_t color)
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
