#include "config.h"

#include "core/config.h"
#include "core/dir.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/game.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/hotkey_config.h"
#include "window/main_menu.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"
#include <string.h>

#define NUM_CHECKBOXES 30
#define CONFIG_PAGES 3
#define NUM_BOTTOM_BUTTONS 6
#define MAX_LANGUAGE_DIRS 20

#define FIRST_BUTTON_Y 72
#define BUTTON_SPACING 24
#define TEXT_Y_OFFSET 4

static int options_per_page[CONFIG_PAGES] = { 7,14,9 };
static void toggle_switch(int id, int param2);
static void button_language_select(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_hotkeys(int param1, int param2);
static void button_close(int save, int param2);
static void button_page(int param1, int param2);
static int config_change_basic(config_key key);
static int config_change_string_basic(config_string_key key);
static int config_change_string_language(config_string_key key);


static generic_button checkbox_buttons[] = {
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_INTRO_VIDEO },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_UI_SMOOTH_SCROLLING },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_UI_WALKER_WAYPOINTS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_CONSTRUCTION_SIZE },
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_100_YEAR_GHOSTS },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GRANDFESTIVAL },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_JEALOUS_GODS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GLOBAL_LABOUR },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_SCHOOL_WALKERS },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_RETIRE_AT_60 },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_FIXED_WORKERS },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_EXTRA_FORTS },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WOLVES_BLOCK },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_DYNAMIC_GRANARIES },
    { 20, 336, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_MORE_STOCKPILE },
    { 20, 360, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_NO_BUYER_DISTRIBUTION },
    { 20, 384, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_EDITOR_EVENTS },
    { 20, 72, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_IMMEDIATELY_DELETE_BUILDINGS },
    { 20, 96, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GRANARIES_GET_DOUBLE },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_FARMS_DELIVER_CLOSE },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_DELIVER_ONLY_TO_ACCEPTING_GRANARIES },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_ALL_HOUSES_MERGE },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WINE_COUNTS_IF_OPEN_TRADE_ROUTE },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY },
};

static generic_button language_button = {
    120, 50, 200, 24, button_language_select, button_none
};



static generic_button bottom_buttons[] = {
    { 200, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
    { 20, 410, 25, 25, button_page, button_none, 0 },
    { 160, 410, 25, 25, button_page, button_none, 1 },
    { 50, 430, 110, 30, button_hotkeys, button_none },


};

static const char* bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK",
    "-",
    "+",
    "Hotkeys"
};

static const char* pages_names[] = {
    "UI Improvements",
    "Gameplay Changes",
    "Gameplay Changes",
};

static const char* option_names[] = {
    "Play intro videos",
    "Extra information in the control panel",
    "Enable smooth scrolling",
    "Draw walker waypoints on overlay after right clicking on a building",
    "Improve visual clarity when clearing",
    "Show range when building reservoirs, fountains and wells",
    "Show draggable construction size",
    "Fix immigration bug on very hard",
    "Fix 100-year-old ghosts",
    "Grand festivals allow extra blessing from a god",
    "Disable jealousness of gods",
    "Enable global labour pool",
    "Extend school walkers range",
    "Change citizens' retirement age from 50 to 60",
    "Fixed worker pool - 38% of population",
    "Allow building 3 extra forts",
    "Block building around wolves",
    "Block unconnected granary roads",
    "Houses stockpile more goods from market",
    "Buying market ladies don't distribute goods",
    "Fix Emperor change and survival time in custom missions",
    "Immediately destroy buildings",
    "Cart pushers from getting granaries can go offroad",
    "Double the capacity of cart pushers from getting granaries",
    "Tower sentries don't need road access from barracks",
    "Farms and wharves deliver only to nearby granaries",
    "Food isn't delivered to getting granaries",
    "All houses merge",
    "Open trade route count as providing different wine type",
    "Randomly collapsing clay pits and iron mines take some money instead",
};

static struct {
    int focus_button;
    int language_focus_button;
    int bottom_focus_button;
    int page;
    int starting_option;
    struct {
        int original_value;
        int new_value;
        int (*change_action)(config_key key);
    } config_values[CONFIG_MAX_ENTRIES];
    struct {
        char original_value[CONFIG_STRING_VALUE_MAX];
        char new_value[CONFIG_STRING_VALUE_MAX];
        int (*change_action)(config_string_key key);
    } config_string_values[CONFIG_STRING_MAX_ENTRIES];
    uint8_t language_options_data[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    uint8_t* language_options[MAX_LANGUAGE_DIRS];
    char language_options_utf8[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    int num_language_options;
    int selected_language_option;
} data;


static void init(void)
{
    data.page = 0;
    data.starting_option = 0;
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        config_key key = checkbox_buttons[i].parameter1;
        data.config_values[i].original_value = config_get(i);
        data.config_values[i].new_value = config_get(i);
        data.config_values[i].change_action = config_change_basic;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; i++) {
        const char* value = config_get_string(i);
        strncpy(data.config_string_values[i].original_value, value, CONFIG_STRING_VALUE_MAX - 1);
        strncpy(data.config_string_values[i].new_value, value, CONFIG_STRING_VALUE_MAX - 1);
    }
    data.config_string_values[0].change_action = config_change_string_language;

    string_copy(string_from_ascii("(default)"), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
    data.language_options[0] = data.language_options_data[0];
    data.num_language_options = 1;
    const dir_listing* subdirs = dir_find_all_subdirectories();
    for (int i = 0; i < subdirs->num_files; i++) {
        if (data.num_language_options < MAX_LANGUAGE_DIRS && lang_dir_is_valid(subdirs->files[i])) {
            int opt_id = data.num_language_options;
            strncpy(data.language_options_utf8[opt_id], subdirs->files[i], CONFIG_STRING_VALUE_MAX - 1);
            encoding_from_utf8(subdirs->files[i], data.language_options_data[opt_id], CONFIG_STRING_VALUE_MAX);
            data.language_options[opt_id] = data.language_options_data[opt_id];
            if (strcmp(data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].original_value, subdirs->files[i]) == 0) {
                data.selected_language_option = opt_id;
            }
            data.num_language_options++;
        }
    }
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(string_from_ascii(pages_names[data.page]), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(string_from_ascii("Language:"), 20, 56, FONT_NORMAL_BLACK, 0);
    text_draw_centered(data.language_options[data.selected_language_option],
        language_button.x, language_button.y + 6, language_button.width, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < options_per_page[data.page]; i++) {
        text_draw(string_from_ascii(option_names[data.starting_option + i]), 44, FIRST_BUTTON_Y + BUTTON_SPACING * i + TEXT_Y_OFFSET, FONT_NORMAL_BLACK, 0);
    }
    for (int i = 0; i < options_per_page[data.page]; i++) {
        int value = i + data.starting_option;
        generic_button* btn = &checkbox_buttons[value];
        if (data.config_values[btn->parameter1].new_value) {
            text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(string_from_ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }
    text_draw_centered(string_from_ascii("Page"), 80, 415, 30, FONT_NORMAL_BLACK, 0);
    text_draw_number(data.page + 1, '@', " ", 120, 415, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}


static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < options_per_page[data.page]; i++) {
        int value = data.starting_option + i;
        generic_button* btn = &checkbox_buttons[value];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == value + 1);
    }
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    button_border_draw(language_button.x, language_button.y, language_button.width, language_button.height, data.language_focus_button == 1);
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    int handled = 0;
    handled |= generic_buttons_min_handle_mouse(m_dialog, 0, 0, checkbox_buttons, data.starting_option + options_per_page[data.page], &data.focus_button, data.starting_option);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, &language_button, 1, &data.language_focus_button);
    if (!handled && (m->right.went_up || h->escape_pressed)) {
        window_main_menu_show(0);
    }
}

static void toggle_switch(int key, int param2)
{
    data.config_values[key].new_value = 1 - data.config_values[key].new_value;
    window_invalidate();
}

static void set_language(int index)
{
    const char* dir = index == 0 ? "" : data.language_options_utf8[index];
    strncpy(data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].new_value, dir, CONFIG_STRING_VALUE_MAX - 1);

    data.selected_language_option = index;
}

static void button_hotkeys(int param1, int param2)
{
    window_hotkey_config_show();
}

static void button_language_select(int param1, int param2)
{
    window_select_list_show_text(
        screen_dialog_offset_x() + language_button.x + language_button.width - 10,
        screen_dialog_offset_y() + language_button.y - 5,
        data.language_options, data.num_language_options, set_language
    );
}

static void button_reset_defaults(int param1, int param2)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; ++i) {
        data.config_values[i].new_value = config_get_default_value(i);
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        strncpy(data.config_string_values[i].new_value, config_get_default_string_value(i), CONFIG_STRING_VALUE_MAX - 1);
    }
    set_language(0);
    window_invalidate();
}

static void cancel_values(void)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; i++) {
        data.config_values[i].new_value = data.config_values[i].original_value;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; i++) {
        strncpy(data.config_string_values[i].new_value, data.config_string_values[i].original_value, CONFIG_STRING_VALUE_MAX - 1);
    }
}

static int config_changed(config_key key)
{
    return data.config_values[key].original_value != data.config_values[key].new_value;
}

static int config_string_changed(config_string_key key)
{
    return strcmp(data.config_string_values[key].original_value, data.config_string_values[key].new_value) != 0;
}

static int apply_changed_configs(void)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; ++i) {
        if (config_changed(i)) {
            if (!data.config_values[i].change_action(i)) {
                return 0;
            }
        }
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        if (config_string_changed(i)) {
            if (!data.config_string_values[i].change_action(i)) {
                return 0;
            }
        }
    }
    return 1;
}

static void button_close(int save, int param2)
{
    if (!save) {
        cancel_values();
        window_main_menu_show(0);
        return;
    }
    if (apply_changed_configs()) {
        window_main_menu_show(0);
    }
}

static void button_page(int param1, int param2)
{
    if (param1) {
        data.page++;
        if (data.page >= CONFIG_PAGES) {
            data.page = 0;
        }
    }
    else {
        data.page--;
        if (data.page < 0) {
            data.page = CONFIG_PAGES - 1;
        }
    }
    data.starting_option = 0;
    for (int i = 0; i < data.page; i++) {
        data.starting_option += options_per_page[i];
    }
    window_invalidate();
}

void window_config_show()
{
    window_type window = {
        WINDOW_CONFIG,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}

static int config_change_basic(config_key key)
{
    config_set(key, data.config_values[key].new_value);
    data.config_values[key].original_value = data.config_values[key].new_value;
    return 1;
}

static int config_change_string_basic(config_string_key key)
{
    config_set_string(key, data.config_string_values[key].new_value);
    strncpy(data.config_string_values[key].original_value, data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}

static int config_change_string_language(config_string_key key)
{
    config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].new_value);
    if (!game_reload_language()) {
        // Notify user that language dir is invalid and revert to previously selected
        window_plain_message_dialog_show(TR_INVALID_LANGUAGE_TITLE, TR_INVALID_LANGUAGE_MESSAGE);
        config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].original_value);
        game_reload_language();
        return 0;
    }
    strncpy(data.config_string_values[key].original_value, data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}
