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
    hotkey_mapping default_mappings[HOTKEY_MAX_ITEMS][2];
    hotkey_mapping mappings[MAX_MAPPINGS];
    int num_mappings;
} data;

static void set_mapping(key_type key, key_modifier_type modifiers, hotkey_action action)
{
    hotkey_mapping *mapping = &data.default_mappings[action][0];
    if (mapping->key) {
        mapping = &data.default_mappings[action][1];
    }
    if (mapping->key) {
        return;
    }
    mapping->key = key;
    mapping->modifiers = modifiers;
    mapping->action = action;
}

static void set_layout_mapping(const char *name, key_type default_key, key_modifier_type modifiers, hotkey_action action)
{
    key_type key = system_keyboard_key_for_symbol(name);
    if (key == KEY_NONE) {
        log_info("No key found on layout for", name, 0);
        key = default_key;
    }
    set_mapping(key, modifiers, action);
}

void init_defaults(void)
{
    memset(data.default_mappings, 0, sizeof(data.default_mappings));
    set_mapping(KEY_UP, KEY_MOD_NONE, HOTKEY_ARROW_UP);
    set_mapping(KEY_DOWN, KEY_MOD_NONE, HOTKEY_ARROW_DOWN);
    set_mapping(KEY_LEFT, KEY_MOD_NONE, HOTKEY_ARROW_LEFT);
    set_mapping(KEY_RIGHT, KEY_MOD_NONE, HOTKEY_ARROW_RIGHT);
    set_layout_mapping("P", KEY_P, KEY_MOD_NONE, HOTKEY_TOGGLE_PAUSE);
    set_mapping(KEY_SPACE, KEY_MOD_NONE, HOTKEY_TOGGLE_OVERLAY);
    set_layout_mapping("L", KEY_L, KEY_MOD_NONE, HOTKEY_CYCLE_LEGION);
    set_layout_mapping("[", KEY_LEFTBRACKET, KEY_MOD_NONE, HOTKEY_DECREASE_GAME_SPEED);
    set_layout_mapping("]", KEY_RIGHTBRACKET, KEY_MOD_NONE, HOTKEY_INCREASE_GAME_SPEED);
    set_mapping(KEY_PAGEDOWN, KEY_MOD_NONE, HOTKEY_DECREASE_GAME_SPEED);
    set_mapping(KEY_PAGEUP, KEY_MOD_NONE, HOTKEY_INCREASE_GAME_SPEED);
    set_mapping(KEY_HOME, KEY_MOD_NONE, HOTKEY_ROTATE_MAP_LEFT);
    set_mapping(KEY_END, KEY_MOD_NONE, HOTKEY_ROTATE_MAP_RIGHT);
    set_mapping(KEY_1, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_LABOR);
    set_mapping(KEY_2, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_MILITARY);
    set_mapping(KEY_3, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_IMPERIAL);
    set_mapping(KEY_4, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RATINGS);
    set_mapping(KEY_5, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_TRADE);
    set_mapping(KEY_6, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_POPULATION);
    set_mapping(KEY_7, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_HEALTH);
    set_mapping(KEY_8, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_EDUCATION);
    set_mapping(KEY_9, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_ENTERTAINMENT);
    set_mapping(KEY_0, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RELIGION);
    set_mapping(KEY_KP_1, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_LABOR);
    set_mapping(KEY_KP_2, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_MILITARY);
    set_mapping(KEY_KP_3, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_IMPERIAL);
    set_mapping(KEY_KP_4, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RATINGS);
    set_mapping(KEY_KP_5, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_TRADE);
    set_mapping(KEY_KP_6, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_POPULATION);
    set_mapping(KEY_KP_7, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_HEALTH);
    set_mapping(KEY_KP_8, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_EDUCATION);
    set_mapping(KEY_KP_9, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_ENTERTAINMENT);
    set_mapping(KEY_KP_0, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_RELIGION);
    set_layout_mapping("-", KEY_MINUS, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_FINANCIAL);
    set_layout_mapping("=", KEY_EQUALS, KEY_MOD_NONE, HOTKEY_SHOW_ADVISOR_CHIEF);
    set_layout_mapping("W", KEY_W, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_WATER);
    set_layout_mapping("F", KEY_F, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_FIRE);
    set_layout_mapping("D", KEY_D, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_DAMAGE);
    set_layout_mapping("C", KEY_C, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_CRIME);
    set_layout_mapping("T", KEY_T, KEY_MOD_NONE, HOTKEY_SHOW_OVERLAY_PROBLEMS);
    set_layout_mapping("A", KEY_A, KEY_MOD_CTRL, HOTKEY_EDITOR_TOGGLE_BATTLE_INFO);
    set_layout_mapping("O", KEY_O, KEY_MOD_CTRL, HOTKEY_LOAD_FILE);
    set_layout_mapping("S", KEY_S, KEY_MOD_CTRL, HOTKEY_SAVE_FILE);
    set_mapping(KEY_F1, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_1);
    set_mapping(KEY_F2, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_2);
    set_mapping(KEY_F3, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_3);
    set_mapping(KEY_F4, KEY_MOD_NONE, HOTKEY_GO_TO_BOOKMARK_4);
    set_mapping(KEY_F1, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_1); // mac specific: F1 key alone does not work
    set_mapping(KEY_F2, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_2);
    set_mapping(KEY_F3, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_3);
    set_mapping(KEY_F4, KEY_MOD_ALT, HOTKEY_GO_TO_BOOKMARK_4);
    set_mapping(KEY_F1, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_1);
    set_mapping(KEY_F2, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_2);
    set_mapping(KEY_F3, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_3);
    set_mapping(KEY_F4, KEY_MOD_CTRL, HOTKEY_SET_BOOKMARK_4);
    set_mapping(KEY_F5, KEY_MOD_NONE, HOTKEY_CENTER_WINDOW);
    set_mapping(KEY_F6, KEY_MOD_NONE, HOTKEY_TOGGLE_FULLSCREEN);
    set_mapping(KEY_ENTER, KEY_MOD_ALT, HOTKEY_TOGGLE_FULLSCREEN);
    set_mapping(KEY_F7, KEY_MOD_NONE, HOTKEY_RESIZE_TO_640);
    set_mapping(KEY_F8, KEY_MOD_NONE, HOTKEY_RESIZE_TO_800);
    set_mapping(KEY_F9, KEY_MOD_NONE, HOTKEY_RESIZE_TO_1024);
    set_mapping(KEY_F12, KEY_MOD_NONE, HOTKEY_SAVE_SCREENSHOT);
    set_mapping(KEY_F12, KEY_MOD_ALT, HOTKEY_SAVE_SCREENSHOT); // mac specific
    set_mapping(KEY_F12, KEY_MOD_CTRL, HOTKEY_SAVE_CITY_SCREENSHOT);
}

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

const hotkey_mapping *hotkey_default_for_action(hotkey_action action, int index)
{
    if (index < 0 || index >= 2 || action < 0 || action >= HOTKEY_MAX_ITEMS) {
        return 0;
    }
    return &data.default_mappings[action][index];
}

void hotkey_config_clear(void)
{
    data.num_mappings = 0;
}

void hotkey_config_add_mapping(const hotkey_mapping *mapping)
{
    if (data.num_mappings < MAX_MAPPINGS) {
        data.mappings[data.num_mappings] = *mapping;
        data.num_mappings++;
    }
}

static void load_defaults(void)
{
    hotkey_config_clear();
    for (int action = 0; action < HOTKEY_MAX_ITEMS; action++) {
        for (int index = 0; index < 2; index++) {
            if (data.default_mappings[action][index].key) {
                hotkey_config_add_mapping(&data.default_mappings[action][index]);
            }
        }
    }
}

static void load_file(void)
{
    hotkey_config_clear();
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
                hotkey_mapping mapping;
                if (key_combination_from_name(value, &mapping.key, &mapping.modifiers)) {
                    mapping.action = i;
                    hotkey_config_add_mapping(&mapping);
                }
                break;
            }
        }
    }
    file_close(fp);
}

void hotkey_config_load(void)
{
    init_defaults();
    load_file();
    if (data.num_mappings == 0) {
        load_defaults();
    }
    hotkey_install_mapping(data.mappings, data.num_mappings);
}

void hotkey_config_save(void)
{
    hotkey_install_mapping(data.mappings, data.num_mappings);
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
