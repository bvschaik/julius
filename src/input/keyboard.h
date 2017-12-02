#ifndef INPUT_KEYBOARD_H
#define INPUT_KEYBOARD_H

#include "graphics/font.h"

#include <stdint.h>

void keyboard_start_capture(uint8_t *text, int max_length, int allow_punctuation, int box_width, font_t font);
void keyboard_refresh();
void keyboard_stop_capture();

int keyboard_input_is_accepted();
int keyboard_is_insert();
int keyboard_cursor_position();

void keyboard_return();

void keyboard_backspace();
void keyboard_delete();
void keyboard_insert();

void keyboard_left();
void keyboard_right();
void keyboard_home();
void keyboard_end();

void keyboard_character(int unicode);


#endif // INPUT_KEYBOARD_H
