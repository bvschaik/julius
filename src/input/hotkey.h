#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

typedef struct {
    int show_advisor;
    int show_overlay;
    int toggle_overlay;
} hotkeys;

const hotkeys *hotkey_state(void);
void hotkey_reset_state(void);

void hotkey_character(int c, int with_ctrl, int with_alt);

void hotkey_left_press(void);
void hotkey_right_press(void);
void hotkey_up_press(void);
void hotkey_down_press(void);
void hotkey_left_release(void);
void hotkey_right_release(void);
void hotkey_up_release(void);
void hotkey_down_release(void);
void hotkey_home(void);
void hotkey_end(void);
void hotkey_esc(void);
void hotkey_page_up(void);
void hotkey_page_down(void);
void hotkey_enter(int with_alt);

void hotkey_func(int f_number, int with_any_modifier, int with_ctrl);

#endif // INPUT_HOTKEY_H
