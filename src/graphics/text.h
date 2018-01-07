#ifndef GRAPHICS_TEXT_H
#define GRAPHICS_TEXT_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

#include <stdint.h>

void Widget_Text_captureCursor(int cursor_position);
void Widget_Text_drawCursor(int xOffset, int yOffset, int isInsert);

int Widget_Text_draw(const uint8_t *str, int x, int y, font_t font, color_t color);

void Widget_Text_drawCentered(const uint8_t *str, int x, int y, int boxWidth, font_t font, color_t color);

int Widget_Text_drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font);
int Widget_Text_drawNumberColored(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font, color_t color);
int Widget_Text_drawMoney(int value, int xOffset, int yOffset, font_t font);
int Widget_Text_drawPercentage(int value, int xOffset, int yOffset, font_t font);

void Widget_Text_drawNumberCentered(int value, int xOffset, int yOffset, int boxWidth, font_t font);
void Widget_Text_drawNumberCenteredColored(int value, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int Widget_Text_drawMultiline(const uint8_t *str, int xOffset, int yOffset, int boxWidth, font_t font);

int Widget_Text_getWidth(const uint8_t *str, font_t font);

#endif // GRAPHICS_TEXT_H
