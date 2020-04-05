#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

typedef struct {
    // fixed keys with multiple functions
    int enter_pressed;
    int escape_pressed;
    // keys with specific function
    int cycle_legion;
    int decrease_game_speed;
    int increase_game_speed;
    int show_advisor;
    int show_overlay;
    int toggle_overlay;
    int toggle_pause;
    int rotate_map_left;
    int rotate_map_right;
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

void hotkey_handle_escape(void);

#endif // INPUT_HOTKEY_H
