#include "lang_text.h"

#include "core/lang.h"
#include "graphics/text.h"

int Widget_GameText_getWidth(int group, int number, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return Widget_Text_getWidth(str, font);
}

int Widget_GameText_getDrawWidth(int group, int number, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return Widget_Text_getWidth(str, font) + font_definition_for(font)->space_width;
}

int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
    return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
    Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font)
{
    int amountOffset = 1;
    if (amount == 1 || amount == -1) {
        amountOffset = 0;
    }
    int descOffsetX;
    if (amount >= 0) {
        descOffsetX = Widget_Text_drawNumber(amount, ' ', " ",
            xOffset, yOffset, font);
    } else {
        descOffsetX = Widget_Text_drawNumber(-amount, '-', " ",
            xOffset, yOffset, font);
    }
    return descOffsetX + Widget_GameText_draw(group, number + amountOffset,
        xOffset + descOffsetX, yOffset, font);
}

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
        width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
    } else {
        width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
        width += Widget_GameText_draw(20, 0, xOffset + width, yOffset, font);
    }
    return width;
}

int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color)
{
    int width = 0;
    if (year >= 0) {
        width += Widget_GameText_drawColored(20, 1, xOffset + width, yOffset, font, color);
        width += Widget_Text_drawNumberColored(year, ' ', " ", xOffset + width, yOffset, font, color);
    } else {
        width += Widget_Text_drawNumberColored(-year, ' ', " ", xOffset + width, yOffset, font, color);
        width += Widget_GameText_drawColored(20, 0, xOffset + width, yOffset, font, color);
    }
    return width;
}

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0) {
        width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
        width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
    } else {
        width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
        width += Widget_GameText_draw(20, 0, xOffset + width - 8, yOffset, font);
    }
    return width;
}

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
    return Widget_Text_drawMultiline(str, xOffset, yOffset, boxWidth, font);
}
