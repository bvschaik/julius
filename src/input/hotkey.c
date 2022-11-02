#include "hotkey.h"

#include "building/type.h"
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
    int save_minimap_screenshot;
} global_hotkeys;

static struct {
    global_hotkeys global_hotkey_state;
    hotkeys hotkey_state;
    hotkey_definition *definitions;
    int num_definitions;
    arrow_definition *arrows;
    int num_arrows;
    int shift_pressed;
} data;

static void set_definition_for_action(hotkey_action action, hotkey_definition *def)
{
    def->value = 1;
    def->repeatable = 0;
    switch (action) {
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
        case HOTKEY_SHOW_ADVISOR_HOUSING:
            def->action = &data.hotkey_state.show_advisor;
            def->value = ADVISOR_HOUSING;
            break;
        case HOTKEY_SHOW_OVERLAY_RELATIVE:
            def->action = &data.hotkey_state.show_overlay_relative;
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
        case HOTKEY_ROTATE_BUILDING:
            def->action = &data.hotkey_state.rotate_building;
            break;
        case HOTKEY_ROTATE_BUILDING_BACK:
            def->action = &data.hotkey_state.rotate_building_back;
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
        case HOTKEY_SAVE_MINIMAP_SCREENSHOT:
            def->action = &data.global_hotkey_state.save_minimap_screenshot;
            break;
        case HOTKEY_BUILD_VACANT_HOUSE:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_HOUSE_VACANT_LOT;
            break;
        case HOTKEY_BUILD_CLEAR_LAND:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_CLEAR_LAND;
            break;
        case HOTKEY_BUILD_ROAD:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_ROAD;
            break;
        case HOTKEY_BUILD_ENGINEERS_POST:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_ENGINEERS_POST;
            break;
        case HOTKEY_BUILD_WALL:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_WALL;
            break;
        case HOTKEY_BUILD_GATEHOUSE:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_GATEHOUSE;
            break;
        case HOTKEY_BUILD_PREFECTURE:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_PREFECTURE;
            break;
        case HOTKEY_BUILD_GRANARY:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_GRANARY;
            break;
        case HOTKEY_BUILD_WAREHOUSE:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_WAREHOUSE;
            break;
        case HOTKEY_BUILD_MARKET:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_MARKET;
            break;
        case HOTKEY_BUILD_PLAZA:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_PLAZA;
            break;
        case HOTKEY_BUILD_GARDENS:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_GARDENS;
            break;
        case HOTKEY_BUILD_RESERVOIR:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_DRAGGABLE_RESERVOIR;
            break;
        case HOTKEY_BUILD_AQUEDUCT:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_AQUEDUCT;
            break;
        case HOTKEY_BUILD_FOUNTAIN:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_FOUNTAIN;
            break;
        case HOTKEY_BUILD_DOCTOR:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_DOCTOR;
            break;
        case HOTKEY_BUILD_BARBER:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_BARBER;
            break;
        case HOTKEY_BUILD_ROADBLOCK:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_ROADBLOCK;
            break;
        case HOTKEY_BUILD_CLONE:
            def->action = &data.hotkey_state.clone_building;
            break;
        case HOTKEY_UNDO:
            def->action = &data.hotkey_state.undo;
            break;
        case HOTKEY_MOTHBALL_TOGGLE:
            def->action = &data.hotkey_state.mothball_toggle;
            break;
        case HOTKEY_STORAGE_ORDER:
            def->action = &data.hotkey_state.storage_order;
            break;
        case HOTKEY_COPY_BUILDING_SETTINGS:
            def->action = &data.hotkey_state.copy_building_settings;
            break;
        case HOTKEY_PASTE_BUILDING_SETTINGS:
            def->action = &data.hotkey_state.paste_building_settings;
            break;
        case HOTKEY_SHOW_OVERLAY_FOOD_STOCKS:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_FOOD_STOCKS;
            break;
        case HOTKEY_SHOW_OVERLAY_ENTERTAINMENT:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_ENTERTAINMENT;
            break;
        case HOTKEY_SHOW_OVERLAY_EDUCATION:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_EDUCATION;
            break;
        case HOTKEY_SHOW_OVERLAY_SCHOOL:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_SCHOOL;
            break;
        case HOTKEY_SHOW_OVERLAY_LIBRARY:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_LIBRARY;
            break;
        case HOTKEY_SHOW_OVERLAY_ACADEMY:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_ACADEMY;
            break;
        case HOTKEY_SHOW_OVERLAY_BARBER:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_BARBER;
            break;
        case HOTKEY_SHOW_OVERLAY_BATHHOUSE:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_BATHHOUSE;
            break;
        case HOTKEY_SHOW_OVERLAY_CLINIC:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_CLINIC;
            break;
        case HOTKEY_SHOW_OVERLAY_HOSPITAL:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_HOSPITAL;
            break;
        case HOTKEY_SHOW_OVERLAY_SICKNESS:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_SICKNESS;
            break;
        case HOTKEY_SHOW_OVERLAY_TAX_INCOME:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_TAX_INCOME;
            break;
        case HOTKEY_SHOW_OVERLAY_DESIRABILITY:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_DESIRABILITY;
            break;
        case HOTKEY_SHOW_OVERLAY_SENTIMENT:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_SENTIMENT;
            break;
        case HOTKEY_SHOW_OVERLAY_MOTHBALL:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_MOTHBALL;
            break;
        case HOTKEY_SHOW_OVERLAY_RELIGION:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_RELIGION;
            break;
        case HOTKEY_SHOW_OVERLAY_ROADS:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_ROADS;
            break;
        case HOTKEY_SHOW_OVERLAY_LEVY:
            def->action = &data.hotkey_state.show_overlay;
            def->value = OVERLAY_LEVY;
            break;
        case HOTKEY_ROTATE_MAP_NORTH:
            def->action = &data.hotkey_state.rotate_map_north;
            break;
        case HOTKEY_BUILD_WHEAT_FARM:
            def->action = &data.hotkey_state.building;
            def->value = BUILDING_WHEAT_FARM;
            break;
        case HOTKEY_SHOW_EMPIRE_MAP:
            def->action = &data.hotkey_state.show_empire_map;
            break;
        case HOTKEY_SHOW_MESSAGES:
            def->action = &data.hotkey_state.show_messages;
            break;

		default:
            def->action = 0;
    }
}

static void add_definition(const hotkey_mapping *mapping)
{
    hotkey_definition *def = &data.definitions[data.num_definitions];
    def->key = mapping->key;
    def->modifiers = mapping->modifiers;
    set_definition_for_action(mapping->action, def);
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
    int total_definitions = 5; // Fixed keys: Enter, ESC, F5, Delete, Backspace
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

    // Fixed keys: Enter, ESC, F5, Delete, Backspace -- yep they're still fixed even down here. crazy, i know
    data.definitions[0].action = &data.hotkey_state.enter_pressed;
    data.definitions[0].key = KEY_TYPE_ENTER;
    data.definitions[0].modifiers = 0;
    data.definitions[0].repeatable = 0;
    data.definitions[0].value = 1;

    data.definitions[1].action = &data.hotkey_state.escape_pressed;
    data.definitions[1].key = KEY_TYPE_ESCAPE;
    data.definitions[1].modifiers = 0;
    data.definitions[1].repeatable = 0;
    data.definitions[1].value = 1;

    data.definitions[2].action = &data.hotkey_state.f5_pressed;
    data.definitions[2].key = KEY_TYPE_F5;
    data.definitions[2].modifiers = 0;
    data.definitions[2].repeatable = 0;
    data.definitions[2].value = 1;

    data.definitions[3].action = &data.hotkey_state.delete_pressed;
    data.definitions[3].key = KEY_TYPE_DELETE;
    data.definitions[3].modifiers = 0;
    data.definitions[3].repeatable = 0;
    data.definitions[3].value = 1;

    data.definitions[4].action = &data.hotkey_state.backspace_pressed;
    data.definitions[4].key = KEY_TYPE_BACKSPACE;
    data.definitions[4].modifiers = 0;
    data.definitions[4].repeatable = 0;
    data.definitions[4].value = 1;

    data.num_definitions = 5;

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
    data.shift_pressed = modifiers == KEY_MOD_SHIFT;

    if (window_is(WINDOW_HOTKEY_EDITOR)) {
        window_hotkey_editor_key_pressed(key, modifiers);
        return;
    }
    if (key == KEY_TYPE_NONE) {
        return;
    }
    int found_action = 0;
    for (int i = 0; i < data.num_definitions; i++) {
        hotkey_definition *def = &data.definitions[i];
        if ((window_is(WINDOW_ASSET_PREVIEWER) || window_is(WINDOW_EDITOR_EMPIRE)) && key == KEY_TYPE_F5 && def->action != &data.hotkey_state.f5_pressed) {
            continue;
        }
        if (def->key == key && def->modifiers == modifiers && (!repeat || def->repeatable)) {
            *(def->action) = def->value;
            found_action = 1;
        }
    }
    if (found_action) {
        return;
    }
    for (int i = 0; i < data.num_arrows; i++) {
        arrow_definition *arrow = &data.arrows[i];
        if (arrow->key == key) {
            arrow->action(1);
        }
    }
}

void hotkey_key_released(key_type key, key_modifier_type modifiers)
{
    data.shift_pressed = modifiers == KEY_MOD_SHIFT;

    if (window_is(WINDOW_HOTKEY_EDITOR)) {
        window_hotkey_editor_key_released(key, modifiers);
        return;
    }
    if (key == KEY_TYPE_NONE) {
        return;
    }
    for (int i = 0; i < data.num_arrows; i++) {
        arrow_definition *arrow = &data.arrows[i];
        if (arrow->key == key) {
            arrow->action(0);
        }
    }
}

int hotkey_shift_pressed(void)
{
    return data.shift_pressed;
}

static void confirm_exit(int accepted, int checked)
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
        graphics_save_screenshot(SCREENSHOT_DISPLAY);
    }
    if (data.global_hotkey_state.save_city_screenshot) {
        graphics_save_screenshot(SCREENSHOT_FULL_CITY);
    }
    if (data.global_hotkey_state.save_minimap_screenshot) {
        graphics_save_screenshot(SCREENSHOT_MINIMAP);
    }
}

void hotkey_set_value_for_action(hotkey_action action, int value)
{
    hotkey_definition def;
    set_definition_for_action(action, &def);
    *(def.action) = value ? def.value : 0;
}
