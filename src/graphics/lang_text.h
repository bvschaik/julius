#ifndef GRAPHICS_LANG_TEXT_H
#define GRAPHICS_LANG_TEXT_H

#include "graphics/font.h"
#include "graphics/color.h"

int lang_text_get_width(int group, int number, font_t font);

int lang_text_draw(int group, int number, int xOffset, int yOffset, font_t font);
int lang_text_draw_colored(int group, int number, int xOffset, int yOffset, font_t font, color_t color);

void lang_text_draw_centered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);
void lang_text_draw_centered_colored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int lang_text_draw_amount(int group, int number, int amount, int xOffset, int yOffset, font_t font);

int lang_text_draw_year(int year, int xOffset, int yOffset, font_t font);
int lang_text_draw_year_colored(int year, int xOffset, int yOffset, font_t font, color_t color);

int lang_text_draw_year_condensed(int year, int xOffset, int yOffset, font_t font);

int lang_text_draw_multiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);

#endif // GRAPHICS_LANG_TEXT_H
