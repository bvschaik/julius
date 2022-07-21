#ifndef GRAPHICS_TEXT_H
#define GRAPHICS_TEXT_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

#include <stdint.h>

void text_capture_cursor(int cursor_position, int offset_start, int offset_end);
void text_draw_cursor(int x_offset, int y_offset, int is_insert);

int text_get_width(const uint8_t *str, font_t font);
int text_get_number_width(int value, char prefix, const char *postfix, font_t font);
unsigned int text_get_max_length_for_width(
    const uint8_t *str, int length, font_t font, unsigned int requested_width, int invert);
void text_ellipsize(uint8_t *str, font_t font, int requested_width);

int text_draw(const uint8_t *str, int x, int y, font_t font, color_t color);
int text_draw_scaled(const uint8_t *str, int x, int y, font_t font, color_t color, float scale);

void text_draw_centered(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color);
int text_draw_ellipsized(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color);

int text_draw_number(int value, char prefix, const char *postfix, int x, int y, font_t font, color_t color);
void text_draw_number_finances(int value, int x, int y, font_t font, color_t color);
int text_draw_number_scaled(int value, char prefix, const char *postfix,
        int x, int y, font_t font, color_t color, float scale);
int text_draw_money(int value, int x_offset, int y_offset, font_t font);
void text_draw_with_money(const uint8_t *text, int value, const char *prefix, const char *postfix,
    int x_offset, int y_offset, int box_width, font_t font, color_t color);
int text_draw_percentage(int value, int x_offset, int y_offset, font_t font);

int text_draw_label_and_number(const uint8_t *label, int value, const char *postfix,
    int x_offset, int y_offset, font_t font, color_t color);
void text_draw_label_and_number_centered(const uint8_t *label, int value, const char *postfix,
    int x_offset, int y_offset, int box_width, font_t font, color_t color);

void text_draw_number_centered(int value, int x_offset, int y_offset, int box_width, font_t font);
void text_draw_number_centered_prefix(
    int value, char prefix, int x_offset, int y_offset, int box_width, font_t font);
void text_draw_number_centered_postfix(
    int value, const char *postfix, int x_offset, int y_offset, int box_width, font_t font);
void text_draw_number_centered_colored(
    int value, int x_offset, int y_offset, int box_width, font_t font, color_t color);

int text_draw_multiline(const uint8_t *str, int x_offset, int y_offset, int box_width, font_t font, uint32_t color);
void text_draw_centered_with_linebreaks(const uint8_t *str, int x, int y, int box_width, font_t font, color_t color);
/**
 * @return Number of lines required to draw the text
 */
int text_measure_multiline(const uint8_t *str, int box_width, font_t font);

#endif // GRAPHICS_TEXT_H
