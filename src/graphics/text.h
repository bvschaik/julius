#ifndef GRAPHICS_TEXT_H
#define GRAPHICS_TEXT_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

#include <stdint.h>

void text_capture_cursor(int cursor_position);
void text_draw_cursor(int xOffset, int yOffset, int isInsert);

int text_get_width(const uint8_t *str, font_t font);

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color);

void text_draw_centered(const uint8_t *str, int x, int y, int boxWidth, font_t font, color_t color);

int text_draw_number(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font);
int text_draw_number_colored(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font, color_t color);
int text_draw_money(int value, int xOffset, int yOffset, font_t font);
int text_draw_percentage(int value, int xOffset, int yOffset, font_t font);

void text_draw_number_centered(int value, int xOffset, int yOffset, int boxWidth, font_t font);
void text_draw_number_centered_colored(int value, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int text_draw_multiline(const uint8_t *str, int xOffset, int yOffset, int boxWidth, font_t font);

#endif // GRAPHICS_TEXT_H
