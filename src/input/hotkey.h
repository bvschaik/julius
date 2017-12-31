#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

void hotkey_character(int c);

void hotkey_left();
void hotkey_right();
void hotkey_up();
void hotkey_down();
void hotkey_home();
void hotkey_end();
void hotkey_esc();

void hotkey_func(int f_number);

void hotkey_ctrl(int is_down);
void hotkey_alt(int is_down);
void hotkey_shift(int is_down);

void hotkey_reset_state();

#endif // INPUT_HOTKEY_H
