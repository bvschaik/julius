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

void lang_text_draw_month_year_max_width(int month, int year, int x_offset, int y_offset, int box_width, font_t font, color_t color)
{
    int month_width = lang_text_get_width(25, month, font);
    int ad_bc_width = lang_text_get_width(20, year >= 0 ? 1 : 0, font);
    int space_width = font_definition_for(font)->space_width;

    int negative_padding = 0;
    // assume 3 digits in the year times 11 pixels plus letter spacing = approx 35px
    int total_width = month_width + ad_bc_width + 35 + 2 * space_width;
    if (total_width > box_width) {
        // take the overflow and divide it by two since we have two places to correct: after month, and after year
        negative_padding = (box_width - total_width) / 2;
        if (negative_padding < -2 * (space_width - 2)) {
            negative_padding = -2 * (space_width - 2);
        }
    }

    int width = negative_padding + lang_text_draw_colored(25, month, x_offset, y_offset, font, color);
    if (year >= 0) {
        int use_year_ad = locale_year_before_ad();
        if (use_year_ad) {
            width += negative_padding + text_draw_number_colored(year, ' ', " ", x_offset + width, y_offset, font, color);
            lang_text_draw_colored(20, 1, x_offset + width, y_offset, font, color);
        } else {
            width += negative_padding + lang_text_draw_colored(20, 1, x_offset + width, y_offset, font, color);
            text_draw_number_colored(year, ' ', " ", x_offset + width, y_offset, font, color);
        }
    } else {
        width += negative_padding + text_draw_number_colored(-year, ' ', " ", x_offset + width, y_offset, font, color);
        lang_text_draw_colored(20, 0, x_offset + width, y_offset, font, color);
    }
}

int lang_text_draw_multiline(int group, int number, int x_offset, int y_offset, int box_width, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return text_draw_multiline(str, x_offset, y_offset, box_width, font, 0);
}
