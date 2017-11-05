#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H

#include "graphics/font.h"

#include <stdint.h>

void KeyboardInput_initTextField(int inputId, uint8_t *text, int maxLength, int textboxWidth, int allowPunctuation, font_t font);

void KeyboardInput_initInput(int inputId);

void KeyboardInput_return();

void KeyboardInput_backspace();
void KeyboardInput_delete();
void KeyboardInput_insert();

void KeyboardInput_left();
void KeyboardInput_right();
void KeyboardInput_home();
void KeyboardInput_end();

void KeyboardInput_character(int c);

#endif
