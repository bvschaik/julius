#include "hotkey_config.h"

#include "core/file.h"
#include "core/log.h"
#include "game/system.h"
#include "input/hotkey.h"

#include <stdio.h>
#include <string.h>

#define MAX_LINE 100
#define MAX_MAPPINGS HOTKEY_MAX_ITEMS * 2

static const char *INI_FILENAME = "julius-hotkeys.ini";

// Keep this in the same order as the actions in hotkey_config.h
static const char *ini_keys[] = {
    "arrow_up",
    "arrow_down",
    "arrow_left",
    "arrow_right",
    "toggle_pause",
    "toggle_overlay",
    "cycle_legion",
    "increase_game_speed",
    "decrease_game_speed",
    "rotate_map_left",
    "rotate_map_right",
    "show_advisor_labor",
    "show_advisor_military",
    "show_advisor_imperial",
    "show_advisor_ratings",
    "show_advisor_trade",
    "show_advisor_population",
    "show_advisor_health",
    "show_advisor_education",
    "show_advisor_entertainment",
    "show_advisor_religion",
    "show_advisor_financial",
    "show_advisor_chief",
    "show_overlay_water",
    "show_overlay_fire",
    "show_overlay_damage",
    "show_overlay_crime",
    "show_overlay_problems",
    "editor_toggle_battle_info",
    "load_file",
    "save_file",
    "go_to_bookmark_1",
    "go_to_bookmark_2",
    "go_to_bookmark_3",
    "go_to_bookmark_4",
    "set_bookmark_1",
    "set_bookmark_2",
    "set_bookmark_3",
    "set_bookmark_4",
    "center_screen",
    "toggle_fullscreen",
    "resize_to_640",
    "resize_to_800",
    "resize_to_1024",
    "save_screenshot",
    "save_city_screenshot",
};

static struct {
    hotkey_mapping mappings[MAX_MAPPINGS];
    int num_mappings;
} data;

const hotkey_mapping *hotkey_for_action(hotkey_action action, int index)
{
    int num = 0;
    for (int i = 0; i < data.num_mappings; i++) {
        if (data.mappings[i].action == action) {
            if (num == index) {
                return &data.mappings[i];
            }
            num++;
        }
    }
    return 0;
}

static void add_mapping(key_type key, key_modifier_type modifiers, hotkey_action action)
{
    data.mappings[data.num_mappings].key = key;
    data.mappings[data.num_mappings].modifiers = modifiers;
    data.mappings[data.num_mappings].action = action;
    data.num_mappings++;
}

static void add_layout_mapping(const char *name, key_type default_key, key_modifier_type modifiers, hotkey_action action)
{
    key_type key = system_keyboard_key_for_symbol(name);
    if (key == KEY_NONE) {
        log_info("No key found on layout for", name, 0);
        key = default_key;
    }
    add_mapping(key, modifiers, action);
}

static void load_defaults(void)
{
    add_mapping(KEY_UP, KEY_MOD_NONE, HOTKEY_ARROW_UP);
    add_mapping(KEY_DOWN, KEY_MOD_NONE, HOTKEY_ARROW_DOWN);
    add_mapping(KEY_LEFT, KEY_MOD_NONE, HOTKEY_ARROW_LEFT);
    add_mapping(KEY_RIGHT, KEY_MOD_NONE, HOTKEY_ARROW_RIGHT);
    add_layout_mapping("P", KEY_P, KEY_MOD_NONE, HOTKEY_TOGGLE_PAUSE);
    add_mapping(KEY_SPACE, KEY_MOD_NONE, HOTKEY_TOGGLE_OVERLAY);
    add_layout_mapping("L", KEY_L, KEY_MOD_NONE, HOTKEY_CYCLE_LEGION);
    add_layout_mapping("[", KEY_LEFTBRACKET, KEY_MOD_NONE, HOTKEY_DECREASE_GAME_SPEED);
    add_layout_mapping("]", KEY_RIGHTBRACKET, KEY_MOD_NONE, HOTKEY_INCREASE_GAME_SPEED);
    add_mapping(KEY_PAGEDOWN, KEY_MOD_NONE, HOTKEY_DECREASE_GAME_SPEED);
    add_mapping(KEY_PAGEUP, KEY_MOD_NONE, HOTKEY_INCREASE_GAME_SPEED);
    add_mapping(KEY_HOME, KEY_MOD_NONE, HOTKEY_ROTATE_MAP_LEFT);
    add_mapping(KEY_END, KEY_MOD_NONE, HOTKEY_ROTATE_MAP_RIGHT);
    add_mapping(KEY_1, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_LABOR);
    add_mapping(KEY_2, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_MILITARY);
    add_mapping(KEY_3, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_IMPERIAL);
    add_mapping(KEY_4, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RATINGS);
    add_mapping(KEY_5, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_TRADE);
    add_mapping(KEY_6, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_POPULATION);
    add_mapping(KEY_7, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_HEALTH);
    add_mapping(KEY_8, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_EDUCATION);
    add_mapping(KEY_9, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_ENTERTAINMENT);
    add_mapping(KEY_0, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RELIGION);
    add_layout_mapping("-", KEY_MINUS, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_FINANCIAL);
    add_layout_mapping("=", KEY_EQUALS, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_CHIEF);
    add_layout_mapping("W", KEY_W, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_WATER);
    add_layout_mapping("F", KEY_F, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_FIRE);
    add_layout_mapping("D", KEY_D, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_DAMAGE);
    add_layout_mapping("C", KEY_C, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_CRIME);
    add_layout_mapping("T", KEY_T, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_PROBLEMS);
    add_layout_mapping("A", KEY_A, KEY_MOD_CTRL, HOTKEY_EDITOR_TOGGLE_BATTLE_INFO);
    add_layout_mapping("O", KEY_O, KEY_MOD_CTRL, HOTKEY_LOAD_FILE);
    add_layout_mapping("S", KEY_S, KEY_MOD_CTRL, HOTKEY_SAVE_FILE);
    add_mapping(KEY_F1, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_1);
    add_mapping(KEY_F2, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_2);
    add_mapping(KEY_F3, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_3);
    add_mapping(KEY_F4, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_4);
    add_mapping(KEY_F1, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_1); // mac specific: F1 key alone does not work
    add_mapping(KEY_F2, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_2);
    add_mapping(KEY_F3, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_3);
    add_mapping(KEY_F4, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_4);
    add_mapping(KEY_F1, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_1);
    add_mapping(KEY_F2, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_2);
    add_mapping(KEY_F3, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_3);
    add_mapping(KEY_F4, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_4);
    add_mapping(KEY_F5, KEY_MOD_NONE, HOTKEY_CENTER_SCREEN);
    add_mapping(KEY_F6, KEY_MOD_NONE, HOTKEY_TOGGLE_FULLSCREEN);
    add_mapping(KEY_ENTER, KEY_MOD_ALT, HOTKEY_TOGGLE_FULLSCREEN);
    add_mapping(KEY_F7, KEY_MOD_NONE, HOTKEY_RESIZE_TO_640);
    add_mapping(KEY_F8, KEY_MOD_NONE, HOTKEY_RESIZE_TO_800);
    add_mapping(KEY_F9, KEY_MOD_NONE, HOTKEY_RESIZE_TO_1024);
    add_mapping(KEY_F12, KEY_MOD_NONE, HOTKEY_SAVE_SCREENSHOT);
    add_mapping(KEY_F12, KEY_MOD_ALT, HOTKEY_SAVE_SCREENSHOT); // mac specific
    add_mapping(KEY_F12, KEY_MOD_CTRL, HOTKEY_SAVE_CITY_SCREENSHOT);
}

static void load_file(void)
{
    FILE *fp = file_open(INI_FILENAME, "rt");
    if (!fp) {
        return;
    }
    char line_buffer[MAX_LINE];
    char *line;
    while ((line = fgets(line_buffer, MAX_LINE, fp))) {
        // Remove newline from string
        size_t last = strlen(line) - 1;
        while (last >= 0 && (line[last] == '\n' || line[last] == '\r')) {
            line[last] = 0;
            last--;
        }
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }
        *equals = 0;
        char *value = &equals[1];
        for (int i = 0; i < HOTKEY_MAX_ITEMS; i++) {
            if (strcmp(ini_keys[i], line) == 0) {
                key_type key;
                key_modifier_type modifiers;
                if (key_combination_from_name(value, &key, &modifiers)) {
                    add_mapping(key, modifiers, i);
                }
                break;
            }
        }
        if (data.num_mappings >= MAX_MAPPINGS) {
            break;
        }
    }
    file_close(fp);
}

void hotkey_config_load(void)
{
    data.num_mappings = 0;
    load_file();
    if (data.num_mappings == 0) {
        load_defaults();
    }
    hotkey_install_mapping(data.mappings, data.num_mappings);
}

void hotkey_config_save(void)
{
    FILE *fp = file_open(INI_FILENAME, "wt");
    if (!fp) {
        log_error("Unable to write hotkey configuration file", INI_FILENAME, 0);
        return;
    }
    for (int i = 0; i < data.num_mappings; i++) {
        const char *key_name = key_combination_name(data.mappings[i].key, data.mappings[i].modifiers);
        fprintf(fp, "%s=%s\n", ini_keys[data.mappings[i].action], key_name);
    }
    file_close(fp);
}
