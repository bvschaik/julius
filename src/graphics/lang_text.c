#include "lang_text.h"

#include "core/lang.h"
#include "core/locale.h"
#include "graphics/text.h"

int lang_text_get_width(int group, int number, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_get_width(str, font) + font_definition_for(font)->space_width;
}

int lang_text_draw(int group, int number, int x_offset, int y_offset, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw(str, x_offset, y_offset, font, 0);
}

int lang_text_draw_colored(int group, int number, int x_offset, int y_offset, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw(str, x_offset, y_offset, font, color);
}

void lang_text_draw_centered(int group, int number, int x_offset, int y_offset, int box_width, font_t font)
{
    const uint8_t* str = lang_get_string(group, number);
    text_draw_centered(str, x_offset, y_offset, box_width, font, 0);
}

void lang_text_draw_centered_colored(int group, int number, int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    const uint8_t* str = lang_get_string(group, number);
    text_draw_centered(str, x_offset, y_offset, box_width, font, color);
}

int lang_text_draw_amount(int group, int number, int amount, int x_offset, int y_offset, font_t font)
{
    int amount_offset = 1;
    if (amount == 1 || amount == -1) {
        amount_offset = 0;
    }
    int desc_offset_x;
    if (amount >= 0) {
        desc_offset_x = text_draw_number(amount, ' ', " ",
            x_offset, y_offset, font);
    } else {
        desc_offset_x = text_draw_number(-amount, '-', " ",
            x_offset, y_offset, font);
    }
    return desc_offset_x + lang_text_draw(group, number + amount_offset,
        x_offset + desc_offset_x, y_offset, font);
}

int lang_text_draw_year(int year, int x_offset, int y_offset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        int use_year_ad = locale_year_before_ad();
        if (use_year_ad) {
            width += text_draw_number(year, ' ', " ", x_offset + width, y_offset, font);
            width += lang_text_draw(20, 1, x_offset + width, y_offset, font);
        } else {
            width += lang_text_draw(20, 1, x_offset + width, y_offset, font);
            width += text_draw_number(year, ' ', " ", x_offset + width, y_offset, font);
        }
    } else {
        width += text_draw_number(-year, ' ', " ", x_offset + width, y_offset, font);
        width += lang_text_draw(20, 0, x_offset + width, y_offset, font);
    }
    return width;
}

int lang_text_draw_year_colored(int year, int x_offset, int y_offset, font_t font, color_t color)
{
    int width = 0;
    if (year >= 0) {
        width += lang_text_draw_colored(20, 1, x_offset + width, y_offset, font, color);
        width += text_draw_number_colored(year, ' ', " ", x_offset + width, y_offset, font, color);
    } else {
        width += text_draw_number_colored(-year, ' ', " ", x_offset + width, y_offset, font, color);
        width += lang_text_draw_colored(20, 0, x_offset + width, y_offset, font, color);
    }
    return width;
}

int lang_text_draw_year_condensed(int year, int x_offset, int y_offset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        width += lang_text_draw(20, 1, x_offset + width, y_offset, font);
        width += text_draw_number(year, ' ', " ", x_offset + width, y_offset, font);
    } else {
        width += text_draw_number(-year, ' ', " ", x_offset + width, y_offset, font);
        width += lang_text_draw(20, 0, x_offset + width - 8, y_offset, font);
    }
    return width;
}

int lang_text_draw_multiline(int group, int number, int x_offset, int y_offset, int box_width, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw_multiline(str, x_offset, y_offset, box_width, font);
}
