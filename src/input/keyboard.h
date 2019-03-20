#ifndef INPUT_KEYBOARD_H
#define INPUT_KEYBOARD_H

#include "graphics/font.h"

#include <stdint.h>

void keyboard_start_capture(uint8_t *text, int max_length, int allow_punctuation, int box_width, font_t font);
void keyboard_refresh(void);
void keyboard_stop_capture(void);

int keyboard_input_is_accepted(void);
int keyboard_is_insert(void);
int keyboard_cursor_position(void);
int keyboard_offset_start(void);
int keyboard_offset_end(void);

void keyboard_return(void);

void keyboard_backspace(void);
void keyboard_delete(void);
void keyboard_insert(void);

void keyboard_left(void);
void keyboard_right(void);
void keyboard_home(void);
void keyboard_end(void);

void keyboard_character(int unicode);


#endif // INPUT_KEYBOARD_H
