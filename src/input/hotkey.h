#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

void hotkey_character(int c);

void hotkey_left(void);
void hotkey_right(void);
void hotkey_up(void);
void hotkey_down(void);
void hotkey_home(void);
void hotkey_end(void);
void hotkey_esc(void);

void hotkey_func(int f_number);

void hotkey_ctrl(int is_down);
void hotkey_alt(int is_down);
void hotkey_shift(int is_down);

void hotkey_reset_state(void);

#endif // INPUT_HOTKEY_H
