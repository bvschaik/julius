#include "hotkey.h"

#include "city/constants.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "graphics/screenshot.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "window/hotkey_editor.h"
#include "window/popup_dialog.h"

#include <stdlib.h>
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

    hotkey_definition *definitions;
    int num_definitions;
    arrow_definition *arrows;
    int num_arrows;    
} data;

static void add_definition(const hotkey_mapping *mapping)
{
    hotkey_definition *def = &data.definitions[data.num_definitions];
    def->key = mapping->key;
    def->modifiers = mapping->modifiers;
    def->value = 1;
    def->repeatable = 0;
    switch (mapping->action) {
        case HOTKEY_TOGGLE_PAUSE:
            def->action = &data.hotkey_state.toggle_pause;
            break;
        case HOTKEY_TOGGLE_OVERLAY:
            def->action = &data.hotkey_state.toggle_overlay;
            break;
        case HOTKEY_CYCLE_LEGION:
            def->action = &data.hotkey_state.cycle_legion;
            break;
        case HOTKEY_INCREASE_GAME_SPEED:
            def->action = &data.hotkey_state.increase_game_speed;
            def->repeatable = 1;
            break;
        case HOTKEY_DECREASE_GAME_SPEED:
            def->action = &data.hotkey_state.decrease_game_speed;
            def->repeatable = 1;
            break;
        case HOTKEY_ROTATE_MAP_LEFT:
            def->action = &data.hotkey_state.rotate_map_left;
            break;
        case HOTKEY_ROTATE_MAP_RIGHT:
            def->action = &data.hotkey_state.rotate_map_right;
            break;
        case HOTKEY_SHOW_ADVISOR_LABOR:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_LABOR;
            break;
        case HOTKEY_SHOW_ADVISOR_MILITARY:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_MILITARY;
            break;
        case HOTKEY_SHOW_ADVISOR_IMPERIAL:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_IMPERIAL;
            break;
        case HOTKEY_SHOW_ADVISOR_RATINGS:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_RATINGS;
            break;
        case HOTKEY_SHOW_ADVISOR_TRADE:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_TRADE;
            break;
        case HOTKEY_SHOW_ADVISOR_POPULATION:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_POPULATION;
            break;
        case HOTKEY_SHOW_ADVISOR_HEALTH:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_HEALTH;
            break;
        case HOTKEY_SHOW_ADVISOR_EDUCATION:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_EDUCATION;
            break;
        case HOTKEY_SHOW_ADVISOR_ENTERTAINMENT:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_ENTERTAINMENT;
            break;
        case HOTKEY_SHOW_ADVISOR_RELIGION:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_RELIGION;
            break;
        case HOTKEY_SHOW_ADVISOR_FINANCIAL:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_FINANCIAL;
            break;
        case HOTKEY_SHOW_ADVISOR_CHIEF:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_CHIEF;
            break;
        case HOTKEY_SHOW_OVERLAY_WATER:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_WATER;
            break;
        case HOTKEY_SHOW_OVERLAY_FIRE:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_FIRE;
            break;
        case HOTKEY_SHOW_OVERLAY_DAMAGE:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_DAMAGE;
            break;
        case HOTKEY_SHOW_OVERLAY_CRIME:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_CRIME;
            break;
        case HOTKEY_SHOW_OVERLAY_PROBLEMS:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_PROBLEMS;
            break;
        case HOTKEY_EDITOR_TOGGLE_BATTLE_INFO:
            def->action = &data.hotkey_state.toggle_editor_battle_info;
            break;
        case HOTKEY_LOAD_FILE:
            def->action = &data.hotkey_state.load_file;
            break;
        case HOTKEY_SAVE_FILE:
            def->action = &data.hotkey_state.save_file;
            break;
        case HOTKEY_GO_TO_BOOKMARK_1:
            def->action = &data.hotkey_state.go_to_bookmark;
            def->value = 1;
            break;
        case HOTKEY_GO_TO_BOOKMARK_2:
            def->action = &data.hotkey_state.go_to_bookmark;
            def->value = 2;
            break;
        case HOTKEY_GO_TO_BOOKMARK_3:
            def->action = &data.hotkey_state.go_to_bookmark;
            def->value = 3;
            break;
        case HOTKEY_GO_TO_BOOKMARK_4:
            def->action = &data.hotkey_state.go_to_bookmark;
            def->value = 4;
            break;
        case HOTKEY_SET_BOOKMARK_1:
            def->action = &data.hotkey_state.set_bookmark;
            def->value = 1;
            break;
        case HOTKEY_SET_BOOKMARK_2:
            def->action = &data.hotkey_state.set_bookmark;
            def->value = 2;
            break;
        case HOTKEY_SET_BOOKMARK_3:
            def->action = &data.hotkey_state.set_bookmark;
            def->value = 3;
            break;
        case HOTKEY_SET_BOOKMARK_4:
            def->action = &data.hotkey_state.set_bookmark;
            def->value = 4;
            break;
        case HOTKEY_CENTER_WINDOW:
            def->action = &data.global_hotkey_state.center_screen;
            break;
        case HOTKEY_TOGGLE_FULLSCREEN:
            def->action = &data.global_hotkey_state.toggle_fullscreen;
            break;
        case HOTKEY_RESIZE_TO_640:
            def->action = &data.global_hotkey_state.resize_to;
            def->value = 640;
            break;
        case HOTKEY_RESIZE_TO_800:
            def->action = &data.global_hotkey_state.resize_to;
            def->value = 800;
            break;
        case HOTKEY_RESIZE_TO_1024:
            def->action = &data.global_hotkey_state.resize_to;
            def->value = 1024;
            break;
        case HOTKEY_SAVE_SCREENSHOT:
            def->action = &data.global_hotkey_state.save_screenshot;
            break;
        case HOTKEY_SAVE_CITY_SCREENSHOT:
            def->action = &data.global_hotkey_state.save_city_screenshot;
            break;
        default:
            def->action = 0;
    }
    if (def->action) {
        data.num_definitions++;
    }
}

static void add_arrow(const hotkey_mapping *mapping)
{
    arrow_definition *arrow = &data.arrows[data.num_arrows];
    arrow->key = mapping->key;
    switch (mapping->action) {
        case HOTKEY_ARROW_UP:
            arrow->action = scroll_arrow_up;
            break;
        case HOTKEY_ARROW_DOWN:
            arrow->action = scroll_arrow_down;
            break;
        case HOTKEY_ARROW_LEFT:
            arrow->action = scroll_arrow_left;
            break;
        case HOTKEY_ARROW_RIGHT:
            arrow->action = scroll_arrow_right;
            break;
        default:
            arrow->action = 0;
            break;
    }
    if (arrow->action) {
        data.num_arrows++;
    }
}

static int allocate_mapping_memory(int total_definitions, int total_arrows)
{
    free(data.definitions);
    free(data.arrows);
    data.num_definitions = 0;
    data.num_arrows = 0;
    data.definitions = malloc(sizeof(hotkey_definition) * total_definitions);
    data.arrows = malloc(sizeof(arrow_definition) * total_arrows);
    if (!data.definitions || !data.arrows) {
        free(data.definitions);
        free(data.arrows);
        return 0;
    }
    return 1;
}

void hotkey_install_mapping(hotkey_mapping *mappings, int num_mappings)
{
    int total_definitions = 2; // Enter and ESC are fixed hotkeys
    int total_arrows = 0;
    for (int i = 0; i < num_mappings; i++) {
        hotkey_action action = mappings[i].action;
        if (action == HOTKEY_ARROW_UP || action == HOTKEY_ARROW_DOWN ||
            action == HOTKEY_ARROW_LEFT || action == HOTKEY_ARROW_RIGHT) {
            total_arrows++;
        } else {
            total_definitions++;
        }
    }
    if (!allocate_mapping_memory(total_definitions, total_arrows)) {
        return;
    }

    // Fixed keys: Escape and Enter
    data.definitions[0].action = &data.hotkey_state.enter_pressed;
    data.definitions[0].key = KEY_ENTER;
    data.definitions[0].modifiers = 0;
    data.definitions[0].repeatable = 0;
    data.definitions[0].value = 1;

    data.definitions[1].action = &data.hotkey_state.escape_pressed;
    data.definitions[1].key = KEY_ESCAPE;
    data.definitions[1].modifiers = 0;
    data.definitions[1].repeatable = 0;
    data.definitions[1].value = 1;

    data.num_definitions = 2;

    for (int i = 0; i < num_mappings; i++) {
        hotkey_action action = mappings[i].action;
        if (action == HOTKEY_ARROW_UP || action == HOTKEY_ARROW_DOWN ||
            action == HOTKEY_ARROW_LEFT || action == HOTKEY_ARROW_RIGHT) {
            add_arrow(&mappings[i]);
        } else {
            add_definition(&mappings[i]);
        }
    }
}

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
    if (window_is(WINDOW_HOTKEY_EDITOR)) {
        window_hotkey_editor_key_pressed(key, modifiers);
        return;
    }
    if (key == KEY_NONE) {
        return;
    }
    for (int i = 0; i < data.num_arrows; i++) {
        arrow_definition *arrow = &data.arrows[i];
        if (arrow->key == key) {
            arrow->action(1);
        }
    }
    for (int i = 0; i < data.num_definitions; i++) {
        hotkey_definition *def = &data.definitions[i];
        if (def->key == key && def->modifiers == modifiers && (!repeat || def->repeatable)) {
            *(def->action) = def->value;
        }
    }
}

void hotkey_key_released(key_type key, key_modifier_type modifiers)
{
    if (window_is(WINDOW_HOTKEY_EDITOR)) {
        window_hotkey_editor_key_released(key, modifiers);
        return;
    }
    if (key == KEY_NONE) {
        return;
    }
    for (int i = 0; i < data.num_arrows; i++) {
        arrow_definition *arrow = &data.arrows[i];
        if (arrow->key == key) {
            arrow->action(0);
        }
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
