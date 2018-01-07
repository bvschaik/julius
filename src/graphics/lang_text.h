#ifndef GRAPHICS_LANG_TEXT_H
#define GRAPHICS_LANG_TEXT_H

#include "graphics/font.h"
#include "graphics/color.h"

int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color);

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);
void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font);

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color);

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font);

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);

int Widget_GameText_getWidth(int group, int number, font_t font);
int Widget_GameText_getDrawWidth(int group, int number, font_t font);

#endif // GRAPHICS_LANG_TEXT_H
