#ifndef KEYBOARDHOTKEY_H
#define KEYBOARDHOTKEY_H

void KeyboardHotkey_character(int c);

void KeyboardHotkey_left();
void KeyboardHotkey_right();
void KeyboardHotkey_up();
void KeyboardHotkey_down();
void KeyboardHotkey_esc();

void KeyboardHotkey_func(int fNumber);

void KeyboardHotkey_ctrl(int isDown);
void KeyboardHotkey_alt(int isDown);
void KeyboardHotkey_shift(int isDown);

void KeyboardHotkey_resetState();

#endif
