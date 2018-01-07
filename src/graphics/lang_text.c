#include "lang_text.h"

#include "core/lang.h"
#include "graphics/text.h"

int lang_text_get_width(int group, int number, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_get_width(str, font) + font_definition_for(font)->space_width;
}

int lang_text_draw(int group, int number, int xOffset, int yOffset, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw(str, xOffset, yOffset, font, 0);
}

int lang_text_draw_colored(int group, int number, int xOffset, int yOffset, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw(str, xOffset, yOffset, font, color);
}

void lang_text_draw_centered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    text_draw_centered(str, xOffset, yOffset, boxWidth, font, 0);
}

void lang_text_draw_centered_colored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
    text_draw_centered(str, xOffset, yOffset, boxWidth, font, color);
}

int lang_text_draw_amount(int group, int number, int amount, int xOffset, int yOffset, font_t font)
{
    int amountOffset = 1;
    if (amount == 1 || amount == -1) {
        amountOffset = 0;
    }
    int descOffsetX;
    if (amount >= 0) {
        descOffsetX = text_draw_number(amount, ' ', " ",
            xOffset, yOffset, font);
    } else {
        descOffsetX = text_draw_number(-amount, '-', " ",
            xOffset, yOffset, font);
    }
    return descOffsetX + lang_text_draw(group, number + amountOffset,
        xOffset + descOffsetX, yOffset, font);
}

int lang_text_draw_year(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        width += lang_text_draw(20, 1, xOffset + width, yOffset, font);
        width += text_draw_number(year, ' ', " ", xOffset + width, yOffset, font);
    } else {
        width += text_draw_number(-year, ' ', " ", xOffset + width, yOffset, font);
        width += lang_text_draw(20, 0, xOffset + width, yOffset, font);
    }
    return width;
}

int lang_text_draw_year_colored(int year, int xOffset, int yOffset, font_t font, color_t color)
{
    int width = 0;
    if (year >= 0) {
        width += lang_text_draw_colored(20, 1, xOffset + width, yOffset, font, color);
        width += text_draw_number_colored(year, ' ', " ", xOffset + width, yOffset, font, color);
    } else {
        width += text_draw_number_colored(-year, ' ', " ", xOffset + width, yOffset, font, color);
        width += lang_text_draw_colored(20, 0, xOffset + width, yOffset, font, color);
    }
    return width;
}

int lang_text_draw_year_condensed(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        width += lang_text_draw(20, 1, xOffset + width, yOffset, font);
        width += text_draw_number(year, ' ', " ", xOffset + width, yOffset, font);
    } else {
        width += text_draw_number(-year, ' ', " ", xOffset + width, yOffset, font);
        width += lang_text_draw(20, 0, xOffset + width - 8, yOffset, font);
    }
    return width;
}

int lang_text_draw_multiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw_multiline(str, xOffset, yOffset, boxWidth, font);
}
