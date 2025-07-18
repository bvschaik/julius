#ifndef TRANSLATION_TRANSLATION_H
#define TRANSLATION_TRANSLATION_H

#include "core/locale.h"

#include <stdint.h>

typedef enum {
    TR_NO_PATCH_TITLE,
    TR_NO_PATCH_MESSAGE,
    TR_MISSING_FONTS_TITLE,
    TR_MISSING_FONTS_MESSAGE,
    TR_NO_EDITOR_TITLE,
    TR_NO_EDITOR_MESSAGE,
    TR_INVALID_LANGUAGE_TITLE,
    TR_INVALID_LANGUAGE_MESSAGE,
    TR_BUILD_ALL_TEMPLES,
    TR_BUTTON_OK,
    TR_BUTTON_CANCEL,
    TR_BUTTON_RESET_DEFAULTS,
    TR_BUTTON_CONFIGURE_HOTKEYS,
    TR_CONFIG_TITLE,
    TR_CONFIG_LANGUAGE_LABEL,
    TR_CONFIG_LANGUAGE_DEFAULT,
    TR_CONFIG_DISPLAY_SCALE,
    TR_CONFIG_CURSOR_SCALE,
    TR_CONFIG_HEADER_UI_CHANGES,
    TR_CONFIG_HEADER_GAMEPLAY_CHANGES,
    TR_CONFIG_SHOW_INTRO_VIDEO,
    TR_CONFIG_SIDEBAR_INFO,
    TR_CONFIG_SMOOTH_SCROLLING,
    TR_CONFIG_DISABLE_MOUSE_EDGE_SCROLLING,
    TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG,
    TR_CONFIG_INVERSE_RIGHT_CLICK_MAP_DRAG,
    TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE,
    TR_CONFIG_ALLOW_CYCLING_TEMPLES,
    TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE,
    TR_CONFIG_SHOW_CONSTRUCTION_SIZE,
    TR_CONFIG_HIGHLIGHT_LEGIONS,
    TR_CONFIG_SHOW_MILITARY_SIDEBAR,
    TR_CONFIG_FIX_IMMIGRATION_BUG,
    TR_CONFIG_FIX_100_YEAR_GHOSTS,
    TR_HOTKEY_TITLE,
    TR_HOTKEY_LABEL,
    TR_HOTKEY_ALTERNATIVE_LABEL,
    TR_HOTKEY_HEADER_ARROWS,
    TR_HOTKEY_HEADER_GLOBAL,
    TR_HOTKEY_HEADER_CITY,
    TR_HOTKEY_HEADER_ADVISORS,
    TR_HOTKEY_HEADER_OVERLAYS,
    TR_HOTKEY_HEADER_BOOKMARKS,
    TR_HOTKEY_HEADER_EDITOR,
    TR_HOTKEY_HEADER_BUILD,
    TR_HOTKEY_ARROW_UP,
    TR_HOTKEY_ARROW_DOWN,
    TR_HOTKEY_ARROW_LEFT,
    TR_HOTKEY_ARROW_RIGHT,
    TR_HOTKEY_TOGGLE_FULLSCREEN,
    TR_HOTKEY_CENTER_WINDOW,
    TR_HOTKEY_RESIZE_TO_640,
    TR_HOTKEY_RESIZE_TO_800,
    TR_HOTKEY_RESIZE_TO_1024,
    TR_HOTKEY_SAVE_SCREENSHOT,
    TR_HOTKEY_SAVE_CITY_SCREENSHOT,
    TR_HOTKEY_BUILD_CLONE,
    TR_HOTKEY_LOAD_FILE,
    TR_HOTKEY_SAVE_FILE,
    TR_HOTKEY_INCREASE_GAME_SPEED,
    TR_HOTKEY_DECREASE_GAME_SPEED,
    TR_HOTKEY_TOGGLE_PAUSE,
    TR_HOTKEY_CYCLE_LEGION,
    TR_HOTKEY_ROTATE_MAP_LEFT,
    TR_HOTKEY_ROTATE_MAP_RIGHT,
    TR_HOTKEY_SHOW_ADVISOR_LABOR,
    TR_HOTKEY_SHOW_ADVISOR_MILITARY,
    TR_HOTKEY_SHOW_ADVISOR_IMPERIAL,
    TR_HOTKEY_SHOW_ADVISOR_RATINGS,
    TR_HOTKEY_SHOW_ADVISOR_TRADE,
    TR_HOTKEY_SHOW_ADVISOR_POPULATION,
    TR_HOTKEY_SHOW_ADVISOR_HEALTH,
    TR_HOTKEY_SHOW_ADVISOR_EDUCATION,
    TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT,
    TR_HOTKEY_SHOW_ADVISOR_RELIGION,
    TR_HOTKEY_SHOW_ADVISOR_FINANCIAL,
    TR_HOTKEY_SHOW_ADVISOR_CHIEF,
    TR_HOTKEY_TOGGLE_OVERLAY,
    TR_HOTKEY_SHOW_OVERLAY_WATER,
    TR_HOTKEY_SHOW_OVERLAY_FIRE,
    TR_HOTKEY_SHOW_OVERLAY_DAMAGE,
    TR_HOTKEY_SHOW_OVERLAY_CRIME,
    TR_HOTKEY_SHOW_OVERLAY_PROBLEMS,
    TR_HOTKEY_GO_TO_BOOKMARK_1,
    TR_HOTKEY_GO_TO_BOOKMARK_2,
    TR_HOTKEY_GO_TO_BOOKMARK_3,
    TR_HOTKEY_GO_TO_BOOKMARK_4,
    TR_HOTKEY_SET_BOOKMARK_1,
    TR_HOTKEY_SET_BOOKMARK_2,
    TR_HOTKEY_SET_BOOKMARK_3,
    TR_HOTKEY_SET_BOOKMARK_4,
    TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO,
    TR_HOTKEY_EDIT_TITLE,
    TR_HOTKEY_DUPLICATE_TITLE,
    TR_HOTKEY_DUPLICATE_MESSAGE,
    TR_WARNING_SCREENSHOT_SAVED,
    TRANSLATION_MAX_KEY
} translation_key;

typedef struct {
    translation_key key;
    const char *string;
} translation_string;

void translation_load(language_type language);

const uint8_t *translation_for(translation_key key);

void translation_czech(const translation_string **strings, int *num_strings);
void translation_english(const translation_string **strings, int *num_strings);
void translation_french(const translation_string **strings, int *num_strings);
void translation_german(const translation_string **strings, int *num_strings);
void translation_greek(const translation_string **strings, int *num_strings);
void translation_italian(const translation_string **strings, int *num_strings);
void translation_japanese(const translation_string **strings, int *num_strings);
void translation_korean(const translation_string **strings, int *num_strings);
void translation_polish(const translation_string **strings, int *num_strings);
void translation_portuguese(const translation_string **strings, int *num_strings);
void translation_russian(const translation_string **strings, int *num_strings);
void translation_spanish(const translation_string **strings, int *num_strings);
void translation_swedish(const translation_string **strings, int *num_strings);
void translation_simplified_chinese(const translation_string **strings, int *num_strings);
void translation_traditional_chinese(const translation_string **strings, int *num_strings);

#endif // TRANSLATION_TRANSLATION_H
