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

#include "window/hotkey_config.h"

#include <string.h>

#define CHECKBOX_CHECK_SIZE 20
#define CHECKBOX_HEIGHT 20
#define CHECKBOX_WIDTH 600

#define NUM_CHECKBOXES 11
#define NUM_BOTTOM_BUTTONS 4
#define MAX_LANGUAGE_DIRS 20

static void toggle_switch(int id);
static void button_language_select(int param1, int param2);
static void button_hotkeys(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

typedef struct {
    int x;
    int y;
    void (*toggle_handler)(int param);
    int parameter;
    translation_key description;
} checkbox;

static checkbox checkbox_buttons[] = {
    { 20, 102, toggle_switch, CONFIG_UI_SHOW_INTRO_VIDEO, TR_CONFIG_SHOW_INTRO_VIDEO },
    { 20, 126, toggle_switch, CONFIG_UI_SIDEBAR_INFO, TR_CONFIG_SIDEBAR_INFO },
    { 20, 150, toggle_switch, CONFIG_UI_SMOOTH_SCROLLING, TR_CONFIG_SMOOTH_SCROLLING },
    { 20, 174, toggle_switch, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE, TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 198, toggle_switch, CONFIG_UI_ALLOW_CYCLING_TEMPLES, TR_CONFIG_ALLOW_CYCLING_TEMPLES },
    { 20, 222, toggle_switch, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE, TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE },
    { 20, 246, toggle_switch, CONFIG_UI_SHOW_CONSTRUCTION_SIZE, TR_CONFIG_SHOW_CONSTRUCTION_SIZE },
    { 20, 270, toggle_switch, CONFIG_UI_HIGHLIGHT_LEGIONS, TR_CONFIG_HIGHLIGHT_LEGIONS },
    { 20, 342, toggle_switch, CONFIG_GP_FIX_IMMIGRATION_BUG, TR_CONFIG_FIX_IMMIGRATION_BUG },
    { 20, 366, toggle_switch, CONFIG_GP_FIX_100_YEAR_GHOSTS, TR_CONFIG_FIX_100_YEAR_GHOSTS },
    { 20, 390, toggle_switch, CONFIG_GP_FIX_EDITOR_EVENTS, TR_CONFIG_FIX_EDITOR_EVENTS }
};

static generic_button language_button = {
    120, 50, 200, 24, button_language_select, button_none
};

static generic_button bottom_buttons[NUM_BOTTOM_BUTTONS] = {
    { 20, 430, 180, 30, button_hotkeys, button_none },
    { 240, 430, 160, 30, button_reset_defaults, button_none },
    { 410, 430, 100, 30, button_close, button_none, 0 },
    { 520, 430, 100, 30, button_close, button_none, 1 },
};

static translation_key bottom_button_texts[] = {
    TR_BUTTON_CONFIGURE_HOTKEYS,
    TR_BUTTON_RESET_DEFAULTS,
    TR_BUTTON_CANCEL,
    TR_BUTTON_OK
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

static void checkboxes_draw_text(const checkbox *checkboxes, int num_checkboxes)
{
    for (int i = 0; i < num_checkboxes; i++) {
        const checkbox *cb = &checkboxes[i];
        if (data.config_values[cb->parameter].new_value) {
            text_draw(string_from_ascii("x"), cb->x + 6, cb->y + 3, FONT_NORMAL_BLACK, 0);
        }
        text_draw(translation_for(cb->description), cb->x + 30, cb->y + 5, FONT_NORMAL_BLACK, 0);
    }
}

static void checkboxes_draw(const checkbox *checkboxes, int num_checkboxes, int focus_button)
{
    for (int i = 0; i < num_checkboxes; i++) {
        const checkbox *cb = &checkboxes[i];
        button_border_draw(cb->x, cb->y, CHECKBOX_CHECK_SIZE, CHECKBOX_CHECK_SIZE, focus_button == i + 1);
    }
}

static int get_checkbox(const mouse *m, const checkbox *checkboxes, int num_checkboxes)
{
    for (int i = 0; i < num_checkboxes; i++) {
        if (checkboxes[i].x <= m->x &&
            checkboxes[i].x + CHECKBOX_WIDTH > m->x &&
            checkboxes[i].y <= m->y &&
            checkboxes[i].y + CHECKBOX_HEIGHT > m->y) {
            return i + 1;
        }
    }
    return 0;
}

static int checkboxes_handle_mouse(const mouse *m, const checkbox *checkboxes, int num_checkboxes, int *focus_button_id)
{
    int checkbox_id = get_checkbox(m, checkboxes, num_checkboxes);
    if (focus_button_id) {
        *focus_button_id = checkbox_id;
    }
    if (!checkbox_id) {
        return 0;
    }
    const checkbox *cb = &checkboxes[checkbox_id - 1];
    if (m->left.went_up) {
        cb->toggle_handler(cb->parameter);
    } else {
        return 0;
    }
    return checkbox_id;
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

    string_copy(translation_for(TR_CONFIG_LANGUAGE_DEFAULT), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
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

    text_draw_centered(translation_for(TR_CONFIG_TITLE), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(translation_for(TR_CONFIG_LANGUAGE_LABEL), 20, 56, FONT_NORMAL_BLACK, 0);
    text_draw_centered(data.language_options[data.selected_language_option],
        language_button.x, language_button.y + 6, language_button.width, FONT_NORMAL_BLACK, 0);

    text_draw(translation_for(TR_CONFIG_HEADER_UI_CHANGES), 20, 83, FONT_NORMAL_BLACK, 0);
    text_draw(translation_for(TR_CONFIG_HEADER_GAMEPLAY_CHANGES), 20, 323, FONT_NORMAL_BLACK, 0);

    checkboxes_draw_text(checkbox_buttons, NUM_CHECKBOXES);

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(translation_for(bottom_button_texts[i]), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    checkboxes_draw(checkbox_buttons, NUM_CHECKBOXES, data.focus_button);

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
    handled |= checkboxes_handle_mouse(m_dialog, checkbox_buttons, NUM_CHECKBOXES, &data.focus_button);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, &language_button, 1, &data.language_focus_button);
    if (!handled && (m->right.went_up || h->escape_pressed)) {
        window_main_menu_show(0);
    }
}

static void toggle_switch(int key)
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

static void button_hotkeys(int param1, int param2)
{
    window_hotkey_config_show();
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
        window_plain_message_dialog_show(TR_INVALID_LANGUAGE_TITLE, TR_INVALID_LANGUAGE_MESSAGE);
        config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].original_value);
        game_reload_language();
        return 0;
    }
    strncpy(data.config_string_values[key].original_value, data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}
