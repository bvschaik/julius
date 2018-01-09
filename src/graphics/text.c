#include "text.h"

#include "core/string.h"
#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/image.h"

static uint8_t tmp_line[200];

static struct {
    int capture;
    int seen;
    int position;
    int cursor_position;
    int width;
    int visible;
    time_millis updated;
    int xOffset;
    int yOffset;
} inputCursor;

void text_capture_cursor(int cursor_position)
{
    inputCursor.capture = 1;
    inputCursor.seen = 0;
    inputCursor.position = 0;
    inputCursor.width = 0;
    inputCursor.cursor_position = cursor_position;
}

void text_draw_cursor(int xOffset, int yOffset, int isInsert)
{
    inputCursor.capture = 0;
    time_millis curr = time_get_millis();
    time_millis diff = curr - inputCursor.updated;
    if (!inputCursor.visible && diff >= 200) {
        inputCursor.visible = 1;
        inputCursor.updated = curr;
    } else if (inputCursor.visible && diff >= 400) {
        inputCursor.visible = 0;
        inputCursor.updated = curr;
    }
    if (inputCursor.visible) {
        if (isInsert) {
            graphics_draw_line(
                xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset - 3,
                xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset - 3,
                COLOR_WHITE);
            graphics_draw_line(
                xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset - 3,
                xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset + 13,
                COLOR_WHITE);
            graphics_draw_line(
                xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset + 14,
                xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset + 14,
                COLOR_WHITE);
        } else {
            graphics_fill_rect(
                xOffset + inputCursor.xOffset, yOffset + inputCursor.yOffset + 14,
                inputCursor.width, 2, COLOR_WHITE);
        }
    }
}

int text_get_width(const uint8_t *str, font_t font)
{
    const font_definition *def = font_definition_for(font);
    int maxlen = 10000;
    int width = 0;
    int graphicBase = image_group(GROUP_FONT);
    while (*str && maxlen > 0) {
        if (*str == ' ') {
            width += def->space_width;
        } else {
            int graphicOffset = font_image_for(*str);
            if (graphicOffset) {
                int graphicId = graphicBase + def->image_offset + graphicOffset - 1;
                width += def->letter_spacing + image_get(graphicId)->width;
            }
        }
        str++;
        maxlen--;
    }
    return width;
}

static int getCharacterWidth(uint8_t c, const font_definition *def)
{
    int graphicOffset = font_image_for(c);
    if (!graphicOffset) {
        return 0;
    }
    int graphicId = image_group(GROUP_FONT) + def->image_offset + graphicOffset - 1;
    return 1 + image_get(graphicId)->width;
}

static int getWordWidth(const uint8_t *str, font_t font, int *outNumChars)
{
    const font_definition *def = font_definition_for(font);
    int width = 0;
    int guard = 0;
    int wordCharSeen = 0;
    int numChars = 0;
    for (uint8_t c = *str; c; c = *(++str)) {
        if (++guard >= 200) {
            break;
        }
        if (c == ' ') {
            if (wordCharSeen) {
                break;
            }
            width += 4;
        } else if (c == '$') {
            if (wordCharSeen) {
                break;
            }
        } else if (c > ' ') {
            // normal char
            width += getCharacterWidth(c, def);
            wordCharSeen = 1;
        }
        numChars++;
    }
    *outNumChars = numChars;
    return width;
}

void text_draw_centered(const uint8_t *str, int x, int y, int boxWidth, font_t font, color_t color)
{
    int offset = (boxWidth - text_get_width(str, font)) / 2;
    if (offset < 0) {
        offset = 0;
    }
    text_draw(str, offset + x, y, font, color);
}

static int drawCharacter(const font_definition *def, unsigned int c, int x, int y, color_t color)
{
    int graphicOffset = font_image_for(c);
    int graphicId = image_group(GROUP_FONT) + def->image_offset + graphicOffset - 1;
    int height = image_get(graphicId)->height - def->line_height;
    if (height < 0) {
        height = 0;
    }
    if (c < 128 || c == 231) { // Some exceptions...
        height = 0;
    }
    image_draw_letter(graphicId, x, y - height, color);
    return image_get(graphicId)->width;
}

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color)
{
    const font_definition *def = font_definition_for(font);

    int currentX = x;
    while (*str) {
        uint8_t c = *str;

        if (c == '_') {
            c = ' ';
        }

        if (c >= ' ') {
            int width = 0;
            int graphic = font_image_for(c);
            if (graphic == 0) {
                width = def->space_width_draw;
            } else {
                width = def->letter_spacing_draw + drawCharacter(def, c, currentX, y, color);
            }
            if (inputCursor.capture && inputCursor.position == inputCursor.cursor_position) {
                if (!inputCursor.seen) {
                    inputCursor.width = width;
                    inputCursor.xOffset = currentX - x;
                    inputCursor.seen = 1;
                }
            }
            currentX += width;
        }

        str++;
        inputCursor.position++;
    }
    if (inputCursor.capture && !inputCursor.seen) {
        inputCursor.width = 4;
        inputCursor.xOffset = currentX - x;
        inputCursor.seen = 1;
    }
    currentX += def->space_width_draw;
    return currentX - x;
}

static void numberToString(uint8_t *str, int value, char prefix, const char *postfix)
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

int text_draw_number(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, prefix, postfix);
    return text_draw(str, xOffset, yOffset, font, 0);
}

int text_draw_number_colored(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font, color_t color)
{
    uint8_t str[100];
    numberToString(str, value, prefix, postfix);
    return text_draw(str, xOffset, yOffset, font, color);
}

int text_draw_money(int value, int xOffset, int yOffset, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, '@', " Dn");
    return text_draw(str, xOffset, yOffset, font, 0);
}

int text_draw_percentage(int value, int xOffset, int yOffset, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, '@', "%");
    return text_draw(str, xOffset, yOffset, font, 0);
}

void text_draw_number_centered(int value, int xOffset, int yOffset, int boxWidth, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, '@', " ");
    text_draw_centered(str, xOffset, yOffset, boxWidth, font, 0);
}

void text_draw_number_centered_colored(int value, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    uint8_t str[100];
    numberToString(str, value, '@', " ");
    text_draw_centered(str, xOffset, yOffset, boxWidth, font, color);
}

int text_draw_multiline(const uint8_t *str, int xOffset, int yOffset, int boxWidth, font_t font)
{
    int lineHeight;
    switch (font) {
        case FONT_LARGE_PLAIN:
        case FONT_LARGE_BLACK:
            lineHeight = 23;
            break;
        case FONT_LARGE_BROWN:
            lineHeight = 24;
            break;
        case FONT_SMALL_PLAIN:
            lineHeight = 9;
            break;
        case FONT_NORMAL_PLAIN:
            lineHeight = 11;
            break;
        default:
            lineHeight = 11;
            break;
    }

    int hasMoreCharacters = 1;
    int guard = 0;
    int y = yOffset;
    while (hasMoreCharacters) {
        if (++guard >= 100) {
            break;
        }
        // clear line
        for (int i = 0; i < 200; i++) {
            tmp_line[i] = 0;
        }
        int currentWidth = 0;
        int lineIndex = 0;
        while (hasMoreCharacters && currentWidth < boxWidth) {
            int wordNumChars;
            int wordWidth = getWordWidth(str, font, &wordNumChars);
            currentWidth += wordWidth;
            if (currentWidth >= boxWidth) {
                if (currentWidth == 0) {
                    hasMoreCharacters = 0;
                }
            } else {
                for (int i = 0; i < wordNumChars; i++) {
                    tmp_line[lineIndex++] = *(str++);
                }
                if (!*str) {
                    hasMoreCharacters = 0;
                }
            }
        }
        text_draw(tmp_line, xOffset, y, font, 0);
        y += lineHeight + 5;
    }
    return y - yOffset;
}
