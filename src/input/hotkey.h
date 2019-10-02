#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

void hotkey_character(int c, int with_ctrl, int with_alt);

void hotkey_left(void);
void hotkey_right(void);
void hotkey_up(void);
void hotkey_down(void);
void hotkey_home(void);
void hotkey_end(void);
void hotkey_esc(void);
void hotkey_page_up(void);
void hotkey_page_down(void);
void hotkey_enter(void);

void hotkey_func(int f_number, int with_modifier);

#endif // INPUT_HOTKEY_H
