#ifndef INPUT_HOTKEY_H
#define INPUT_HOTKEY_H

#include "core/hotkey_config.h"
#include "input/keys.h"

typedef struct {
    // fixed keys with multiple functions
    int enter_pressed;
    int escape_pressed;
    // keys with specific function
    int cycle_legion;
    int decrease_game_speed;
    int increase_game_speed;
    int rotate_map_left;
    int rotate_map_right;
    int show_advisor;
    int show_overlay;
    int toggle_overlay;
    int toggle_pause;
    int toggle_editor_battle_info;
    int set_bookmark;
    int go_to_bookmark;
    int load_file;
    int save_file;
} hotkeys;

void hotkey_install_mapping(hotkey_mapping *mappings, int num_mappings);

const hotkeys *hotkey_state(void);
void hotkey_reset_state(void);

void hotkey_key_pressed(key_type key, key_modifier_type modifiers, int repeat);
void hotkey_key_released(key_type key, key_modifier_type modifiers);

void hotkey_handle_escape(void);

void hotkey_handle_global_keys(void);

#endif // INPUT_HOTKEY_H
