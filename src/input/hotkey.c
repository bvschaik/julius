#include "hotkey.h"

#include "city/constants.h"
#include "game/cheats.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "graphics/screenshot.h"
#include "graphics/video.h"
#include "input/scroll.h"
#include "window/popup_dialog.h"

#include <string.h>

typedef struct {
    int *action;
    int value;
    key_type key;
    key_modifier_type modifiers;
    int repeatable;
} hotkey_definition;

typedef struct {
    void (*action)(int is_down);
    key_type key;
} arrow_definition;

typedef struct {
    int center_screen;
    int toggle_fullscreen;
    int resize_to;
    int save_screenshot;
    int save_city_screenshot;
} global_hotkeys;

static struct {
    global_hotkeys global_hotkey_state;
    hotkeys hotkey_state;
} data;

static hotkey_definition definitions[] = {
    { &data.hotkey_state.enter_pressed, 1, KEY_ENTER, KEY_MOD_NONE },
    { &data.hotkey_state.escape_pressed, 1, KEY_ESCAPE, KEY_MOD_NONE },
    { &data.hotkey_state.toggle_pause, 1, KEY_P, KEY_MOD_NONE },
    { &data.hotkey_state.toggle_overlay, 1, KEY_SPACE, KEY_MOD_NONE },
    { &data.hotkey_state.cycle_legion, 1, KEY_L, KEY_MOD_NONE },
    { &data.hotkey_state.decrease_game_speed, 1, KEY_LEFTBRACKET, KEY_MOD_NONE, 1 },
    { &data.hotkey_state.increase_game_speed, 1, KEY_RIGHTBRACKET, KEY_MOD_NONE, 1 },
    { &data.hotkey_state.rotate_map_left, 1, KEY_HOME, KEY_MOD_NONE },
    { &data.hotkey_state.rotate_map_right, 1, KEY_END, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_LABOR, KEY_1, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_MILITARY, KEY_2, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_IMPERIAL, KEY_3, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_RATINGS, KEY_4, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_TRADE, KEY_5, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_POPULATION, KEY_6, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_HEALTH, KEY_7, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_EDUCATION, KEY_8, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_ENTERTAINMENT, KEY_9, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_RELIGION, KEY_0, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_FINANCIAL, KEY_MINUS, KEY_MOD_NONE },
    { &data.hotkey_state.show_advisor, ADVISOR_CHIEF, KEY_EQUALS, KEY_MOD_NONE },
    { &data.hotkey_state.show_overlay, OVERLAY_WATER, KEY_W, KEY_MOD_NONE },
    { &data.hotkey_state.show_overlay, OVERLAY_FIRE, KEY_F, KEY_MOD_NONE },
    { &data.hotkey_state.show_overlay, OVERLAY_DAMAGE, KEY_D, KEY_MOD_NONE },
    { &data.hotkey_state.show_overlay, OVERLAY_CRIME, KEY_C, KEY_MOD_NONE },
    { &data.hotkey_state.show_overlay, OVERLAY_PROBLEMS, KEY_T, KEY_MOD_NONE },
    { &data.hotkey_state.toggle_editor_battle_info, 1, KEY_A, KEY_MOD_CTRL },
    { &data.hotkey_state.load_file, 1, KEY_O, KEY_MOD_CTRL },
    { &data.hotkey_state.save_file, 1, KEY_S, KEY_MOD_CTRL },
    { &data.hotkey_state.go_to_bookmark, 1, KEY_F1, KEY_MOD_NONE },
    { &data.hotkey_state.go_to_bookmark, 2, KEY_F2, KEY_MOD_NONE },
    { &data.hotkey_state.go_to_bookmark, 3, KEY_F3, KEY_MOD_NONE },
    { &data.hotkey_state.go_to_bookmark, 4, KEY_F4, KEY_MOD_NONE },
    { &data.hotkey_state.set_bookmark, 1, KEY_F1, KEY_MOD_CTRL },
    { &data.hotkey_state.set_bookmark, 2, KEY_F2, KEY_MOD_CTRL },
    { &data.hotkey_state.set_bookmark, 3, KEY_F3, KEY_MOD_CTRL },
    { &data.hotkey_state.set_bookmark, 4, KEY_F4, KEY_MOD_CTRL },
    { &data.hotkey_state.set_bookmark, 1, KEY_F1, KEY_MOD_ALT }, // mac specific: F1 key alone does not work
    { &data.hotkey_state.set_bookmark, 2, KEY_F2, KEY_MOD_ALT },
    { &data.hotkey_state.set_bookmark, 3, KEY_F3, KEY_MOD_ALT },
    { &data.hotkey_state.set_bookmark, 4, KEY_F4, KEY_MOD_ALT },
    { &data.global_hotkey_state.center_screen, 1, KEY_F5, KEY_MOD_NONE },
    { &data.global_hotkey_state.toggle_fullscreen, 1, KEY_F6, KEY_MOD_NONE },
    { &data.global_hotkey_state.toggle_fullscreen, 1, KEY_ENTER, KEY_MOD_ALT },
    { &data.global_hotkey_state.resize_to, 640, KEY_F7, KEY_MOD_NONE },
    { &data.global_hotkey_state.resize_to, 800, KEY_F8, KEY_MOD_NONE },
    { &data.global_hotkey_state.resize_to, 1024, KEY_F9, KEY_MOD_NONE },
    { &data.global_hotkey_state.save_screenshot, 1, KEY_F12, KEY_MOD_NONE },
    { &data.global_hotkey_state.save_screenshot, 1, KEY_F12, KEY_MOD_ALT }, // mac specific
    { &data.global_hotkey_state.save_city_screenshot, 1, KEY_F12, KEY_MOD_CTRL },
    { 0 }
};

static arrow_definition arrows[] = {
    { scroll_arrow_up, KEY_UP },
    { scroll_arrow_down, KEY_DOWN },
    { scroll_arrow_left, KEY_LEFT },
    { scroll_arrow_right, KEY_RIGHT },
    { 0 }
};

const hotkeys *hotkey_state(void)
{
    return &data.hotkey_state;
}

void hotkey_reset_state(void)
{
    memset(&data.hotkey_state, 0, sizeof(data.hotkey_state));
    memset(&data.global_hotkey_state, 0, sizeof(data.global_hotkey_state));
}

void hotkey_key_pressed(key_type key, key_modifier_type modifiers, int repeat)
{
    arrow_definition *arrow = arrows;
    while (arrow->action) {
        if (arrow->key == key) {
            arrow->action(1);
        }
        arrow++;
    }
    hotkey_definition *def = definitions;
    while (def->action) {
        if (def->key == key && def->modifiers == modifiers && (!repeat || def->repeatable)) {
            *(def->action) = def->value;
        }
        def++;
    }
}

void hotkey_key_released(key_type key)
{
    arrow_definition *arrow = arrows;
    while (arrow->action) {
        if (arrow->key == key) {
            arrow->action(0);
        }
        arrow++;
    }
}

void hotkey_character(int c, int with_ctrl, int with_alt)
{
    if (with_alt) {
        switch (c) {
            case 'k':
                game_cheat_activate();
                break;
            case 'c':
                game_cheat_money();
                break;
            case 'v':
                game_cheat_victory();
                break;
        }
        return;
    }
}

static void confirm_exit(int accepted)
{
    if (accepted) {
        system_exit();
    }
}

void hotkey_handle_escape(void)
{
    video_stop();
    window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
}

void hotkey_handle_global_keys(void)
{
    if (data.global_hotkey_state.center_screen) {
        system_center();
    }
    if (data.global_hotkey_state.resize_to) {
        switch (data.global_hotkey_state.resize_to) {
            case 640: system_resize(640, 480); break;
            case 800: system_resize(800, 600); break;
            case 1024: system_resize(1024, 768); break;
        }
    }
    if (data.global_hotkey_state.toggle_fullscreen) {
        system_set_fullscreen(!setting_fullscreen());
    }
    if (data.global_hotkey_state.save_screenshot) {
        graphics_save_screenshot(0);
    }
    if (data.global_hotkey_state.save_city_screenshot) {
        graphics_save_screenshot(1);
    }
}
