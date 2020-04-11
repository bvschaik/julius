#ifndef CORE_HOTKEY_CONFIG_H
#define CORE_HOTKEY_CONFIG_H

#include "input/keys.h"

typedef enum {
    HOTKEY_ARROW_UP,
    HOTKEY_ARROW_DOWN,
    HOTKEY_ARROW_LEFT,
    HOTKEY_ARROW_RIGHT,
    HOTKEY_TOGGLE_PAUSE,
    HOTKEY_TOGGLE_OVERLAY,
    HOTKEY_CYCLE_LEGION,
    HOTKEY_INCREASE_GAME_SPEED,
    HOTKEY_DECREASE_GAME_SPEED,
    HOTKEY_ROTATE_MAP_LEFT,
    HOTKEY_ROTATE_MAP_RIGHT,
    HOTKEY_SHOW_ADVISOR_LABOR,
    HOTKEY_SHOW_ADVISOR_MILITARY,
    HOTKEY_SHOW_ADVISOR_IMPERIAL,
    HOTKEY_SHOW_ADVISOR_RATINGS,
    HOTKEY_SHOW_ADVISOR_TRADE,
    HOTKEY_SHOW_ADVISOR_POPULATION,
    HOTKEY_SHOW_ADVISOR_HEALTH,
    HOTKEY_SHOW_ADVISOR_EDUCATION,
    HOTKEY_SHOW_ADVISOR_ENTERTAINMENT,
    HOTKEY_SHOW_ADVISOR_RELIGION,
    HOTKEY_SHOW_ADVISOR_FINANCIAL,
    HOTKEY_SHOW_ADVISOR_CHIEF,
    HOTKEY_SHOW_OVERLAY_WATER,
    HOTKEY_SHOW_OVERLAY_FIRE,
    HOTKEY_SHOW_OVERLAY_DAMAGE,
    HOTKEY_SHOW_OVERLAY_CRIME,
    HOTKEY_SHOW_OVERLAY_PROBLEMS,
    HOTKEY_EDITOR_TOGGLE_BATTLE_INFO,
    HOTKEY_LOAD_FILE,
    HOTKEY_SAVE_FILE,
    HOTKEY_GO_TO_BOOKMARK_1,
    HOTKEY_GO_TO_BOOKMARK_2,
    HOTKEY_GO_TO_BOOKMARK_3,
    HOTKEY_GO_TO_BOOKMARK_4,
    HOTKEY_SET_BOOKMARK_1,
    HOTKEY_SET_BOOKMARK_2,
    HOTKEY_SET_BOOKMARK_3,
    HOTKEY_SET_BOOKMARK_4,
    HOTKEY_CENTER_SCREEN,
    HOTKEY_TOGGLE_FULLSCREEN,
    HOTKEY_RESIZE_TO_640,
    HOTKEY_RESIZE_TO_800,
    HOTKEY_RESIZE_TO_1024,
    HOTKEY_SAVE_SCREENSHOT,
    HOTKEY_SAVE_CITY_SCREENSHOT,
    HOTKEY_MAX_ITEMS
} hotkey_action;

typedef struct {
    key_type key;
    key_modifier_type modifiers;
    hotkey_action action;
} hotkey_mapping;

/**
 * Load hotkey config from file
 */
void hotkey_config_load(void);

/**
 * Save hotkey config to file
 */
void hotkey_config_save(void);

#endif // CORE_HOTKEY_CONFIG_H
