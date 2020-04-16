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
#include "window/main_menu.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

#include <string.h>

#define NUM_CHECKBOXES 10
#define NUM_BOTTOM_BUTTONS 3
#define MAX_LANGUAGE_DIRS 20

static void toggle_switch(int id, int param2);
static void button_language_select(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static generic_button checkbox_buttons[] = {
    { 20, 102, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_INTRO_VIDEO },
    { 20, 126, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO },
    { 20, 150, 20, 20, toggle_switch, button_none, CONFIG_UI_SMOOTH_SCROLLING },
    { 20, 174, 20, 20, toggle_switch, button_none, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 198, 20, 20, toggle_switch, button_none, CONFIG_UI_ALLOW_CYCLING_TEMPLES },
    { 20, 222, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE },
    { 20, 246, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_CONSTRUCTION_SIZE },
    { 20, 318, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG },
    { 20, 342, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_100_YEAR_GHOSTS },
    { 20, 366, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_EDITOR_EVENTS }
};

static generic_button language_button = {
    120, 50, 200, 24, button_language_select, button_none
};

static generic_button bottom_buttons[] = {
    { 20, 430, 150, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
};

static const char *bottom_button_texts[] = {
    "Reset defaults",
    "Cancel",
    "OK"
};

static struct {
    int focus_button;
    int language_focus_button;
    int bottom_focus_button;
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
    uint8_t *language_options[MAX_LANGUAGE_DIRS];
    char language_options_utf8[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    int num_language_options;
    int selected_language_option;
} data;

static int config_change_basic(config_key key);
static int config_change_string_basic(config_string_key key);
static int config_change_string_language(config_string_key key);

static void init_config_values(void)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; ++i) {
        data.config_values[i].change_action = config_change_basic;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        data.config_string_values[i].change_action = config_change_string_basic;
    }
    data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].change_action = config_change_string_language;
}

static void init(void)
{
    if (!data.config_values[0].change_action) {
        init_config_values();
    }
    for (int i = 0; i < CONFIG_MAX_ENTRIES; i++) {
        data.config_values[i].original_value = config_get(i);
        data.config_values[i].new_value = config_get(i);
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; i++) {
        const char *value = config_get_string(i);
        strncpy(data.config_string_values[i].original_value, value, CONFIG_STRING_VALUE_MAX - 1);
        strncpy(data.config_string_values[i].new_value, value, CONFIG_STRING_VALUE_MAX - 1);
    }

    string_copy(string_from_ascii("(default)"), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
    data.language_options[0] = data.language_options_data[0];
    data.num_language_options = 1;
    const dir_listing *subdirs = dir_find_all_subdirectories();
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

    text_draw_centered(string_from_ascii("Julius configuration options"), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(string_from_ascii("Language:"), 20, 56, FONT_NORMAL_BLACK, 0);
    text_draw_centered(data.language_options[data.selected_language_option],
        language_button.x, language_button.y + 6, language_button.width, FONT_NORMAL_BLACK, 0);

    text_draw(string_from_ascii("User interface changes"), 20, 83, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Play intro videos"), 50, 107, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Extra information in the control panel"), 50, 131, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Enable smooth scrolling"), 50, 155, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Improve visual feedback when clearing land"), 50, 179, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Allow building each temple in succession"), 50, 203, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Show range when building reservoirs, fountains and wells"), 50, 227, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Show draggable construction size"), 50, 251, FONT_NORMAL_BLACK, 0);
  
    text_draw(string_from_ascii("Gameplay changes"), 20, 299, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix immigration bug on very hard"), 50, 323, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix 100-year-old ghosts"), 50, 347, FONT_NORMAL_BLACK, 0);
    text_draw(string_from_ascii("Fix Emperor change and survival time in custom missions"), 50, 371, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        if (data.config_values[btn->parameter1].new_value) {
            text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(string_from_ascii(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    for (int i = 0; i < NUM_CHECKBOXES; i++) {
        generic_button *btn = &checkbox_buttons[i];
        button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == i + 1);
    }
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    button_border_draw(language_button.x, language_button.y, language_button.width, language_button.height, data.language_focus_button == 1);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    int handled = 0;
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, checkbox_buttons, NUM_CHECKBOXES, &data.focus_button);
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
    const char *dir = index == 0 ? "" : data.language_options_utf8[index];
    strncpy(data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].new_value, dir, CONFIG_STRING_VALUE_MAX - 1);

    data.selected_language_option = index;
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
        window_plain_message_dialog_show(
            "Invalid language directory",
            "The directory you selected does not contain a valid language pack. Please check the log for errors."
        );
        config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].original_value);
        game_reload_language();
        return 0;
    }
    strncpy(data.config_string_values[key].original_value, data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}
