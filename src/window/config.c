#include "config.h"

#include "core/config.h"
#include "core/dir.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/game.h"
#include "game/system.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/hotkey_config.h"
#include "window/main_menu.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"

#include <string.h>

#define CHECKBOX_CHECK_SIZE 20
#define CHECKBOX_HEIGHT 20
#define CHECKBOX_WIDTH 560
#define CHECKBOX_TEXT_WIDTH CHECKBOX_WIDTH - CHECKBOX_CHECK_SIZE - 15

#define NUM_VISIBLE_ITEMS 15

#define ITEM_Y_OFFSET 60
#define ITEM_HEIGHT 24

#define NUM_BOTTOM_BUTTONS 4
#define MAX_LANGUAGE_DIRS 20

static void on_scroll(void);

static void toggle_switch(int id);
static void button_language_select(int param1, int param2);
static void button_display_scale_select(int param1, int param2);
static void button_cursor_scale_select(int param1, int param2);
static void button_hotkeys(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static const uint8_t *display_text_language(void);
static const uint8_t *display_text_display_scale(void);
static const uint8_t *display_text_cursor_scale(void);

static scrollbar_type scrollbar = {580, ITEM_Y_OFFSET, ITEM_HEIGHT * NUM_VISIBLE_ITEMS, on_scroll, 4};

enum {
    TYPE_NONE,
    TYPE_HEADER,
    TYPE_CHECKBOX,
    TYPE_SELECT
};

enum {
    SELECT_LANGUAGE,
    SELECT_DISPLAY,
    SELECT_CURSOR
};

typedef struct {
    int type;
    translation_key description;
    int checkbox_parameter;
    int select_type;
    const uint8_t* (*get_display_text)(void);
} config_widget;

static config_widget widgets[] = {
    { TYPE_SELECT, TR_CONFIG_LANGUAGE_LABEL, 0,  SELECT_LANGUAGE, display_text_language},
    { TYPE_SELECT, TR_CONFIG_DISPLAY_SCALE, 0, SELECT_DISPLAY, display_text_display_scale},
    { TYPE_SELECT, TR_CONFIG_CURSOR_SCALE, 0, SELECT_CURSOR, display_text_cursor_scale},
    { TYPE_NONE },
    { TYPE_HEADER, TR_CONFIG_HEADER_UI_CHANGES },
    { TYPE_CHECKBOX, TR_CONFIG_SHOW_INTRO_VIDEO, CONFIG_UI_SHOW_INTRO_VIDEO},
    { TYPE_CHECKBOX, TR_CONFIG_SIDEBAR_INFO, CONFIG_UI_SIDEBAR_INFO},
    { TYPE_CHECKBOX, TR_CONFIG_SMOOTH_SCROLLING, CONFIG_UI_SMOOTH_SCROLLING },
    { TYPE_CHECKBOX, TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG, CONFIG_UI_DISABLE_RIGHT_CLICK_MAP_DRAG },
    { TYPE_CHECKBOX, TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE },
    { TYPE_CHECKBOX, TR_CONFIG_ALLOW_CYCLING_TEMPLES, CONFIG_UI_ALLOW_CYCLING_TEMPLES },
    { TYPE_CHECKBOX, TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE },
    { TYPE_CHECKBOX, TR_CONFIG_SHOW_CONSTRUCTION_SIZE, CONFIG_UI_SHOW_CONSTRUCTION_SIZE },
    { TYPE_CHECKBOX, TR_CONFIG_HIGHLIGHT_LEGIONS, CONFIG_UI_HIGHLIGHT_LEGIONS },
    { TYPE_CHECKBOX, TR_CONFIG_SHOW_MILITARY_SIDEBAR, CONFIG_UI_SHOW_MILITARY_SIDEBAR },
    { TYPE_NONE },
    { TYPE_HEADER, TR_CONFIG_HEADER_GAMEPLAY_CHANGES },
    { TYPE_CHECKBOX, TR_CONFIG_FIX_IMMIGRATION_BUG, CONFIG_GP_FIX_IMMIGRATION_BUG },
    { TYPE_CHECKBOX, TR_CONFIG_FIX_100_YEAR_GHOSTS, CONFIG_GP_FIX_100_YEAR_GHOSTS }
};

static generic_button select_buttons[] = {
    { 150, 0, 200, 24, button_language_select, button_none },
    { 150, 0, 200, 24, button_display_scale_select, button_none },
    { 150, 0, 200, 24, button_cursor_scale_select, button_none },
};

static generic_button bottom_buttons[NUM_BOTTOM_BUTTONS] = {
    { 20, 430, 180, 30, button_hotkeys, button_none },
    { 230, 430, 180, 30, button_reset_defaults, button_none },
    { 415, 430, 100, 30, button_close, button_none, 0 },
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
static int config_change_display_scale(config_key key);
static int config_change_cursor_scale(config_key key);
static int config_change_string_basic(config_string_key key);
static int config_change_string_language(config_string_key key);

static uint8_t *percentage_string(uint8_t *string, int percentage)
{
    int offset = string_from_int(string, percentage, 0);
    string[offset] = '%';
    string[offset + 1] = 0;
    return string;
}

static const uint8_t *display_text_language(void)
{
    return data.language_options[data.selected_language_option];
}

static const uint8_t *display_text_display_scale(void)
{
    static uint8_t value[10];
    return percentage_string(value, data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].new_value);
}

static const uint8_t *display_text_cursor_scale(void)
{
    static uint8_t value[10];
    return percentage_string(value, data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value);
}

static void init_config_values(void)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; ++i) {
        data.config_values[i].change_action = config_change_basic;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        data.config_string_values[i].change_action = config_change_string_basic;
    }
    data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].change_action = config_change_display_scale;
    data.config_values[CONFIG_SCREEN_CURSOR_SCALE].change_action = config_change_cursor_scale;
    data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].change_action = config_change_string_language;
}

static void checkbox_draw_text(int x, int y, int value_key, translation_key description)
{
    if (data.config_values[value_key].new_value) {
        text_draw(string_from_ascii("x"), x + 6, y + 3, FONT_NORMAL_BLACK, 0);
    }
    text_draw_ellipsized(translation_for(description), x + 30, y + 5, CHECKBOX_TEXT_WIDTH, FONT_NORMAL_BLACK, 0);
}

static void checkbox_draw(int x, int y, int has_focus)
{
    button_border_draw(x, y, CHECKBOX_CHECK_SIZE, CHECKBOX_CHECK_SIZE, has_focus);
}

static int is_checkbox(const mouse *m, int x, int y)
{
    if (x <= m->x && x + CHECKBOX_WIDTH > m->x &&
        y <= m->y && y + CHECKBOX_HEIGHT > m->y) {
        return 1;
    }
    return 0;
}

static int checkbox_handle_mouse(const mouse *m, int x, int y, int value_key, int *focus)
{
    if (!is_checkbox(m, x, y)) {
        return 0;
    }
    *focus = 1;
    if (m->left.went_up) {
        toggle_switch(value_key);
        return 1;
    } else {
        return 0;
    }
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
    const char *original_value = data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].original_value;
    for (int i = 0; i < subdirs->num_files; i++) {
        if (data.num_language_options < MAX_LANGUAGE_DIRS && lang_dir_is_valid(subdirs->files[i])) {
            int opt_id = data.num_language_options;
            strncpy(data.language_options_utf8[opt_id], subdirs->files[i], CONFIG_STRING_VALUE_MAX - 1);
            encoding_from_utf8(subdirs->files[i], data.language_options_data[opt_id], CONFIG_STRING_VALUE_MAX);
            data.language_options[opt_id] = data.language_options_data[opt_id];
            if (strcmp(original_value, subdirs->files[i]) == 0) {
                data.selected_language_option = opt_id;
            }
            data.num_language_options++;
        }
    }

    scrollbar_init(&scrollbar, 0, sizeof(widgets) / sizeof(config_widget) - NUM_VISIBLE_ITEMS);
}

static void draw_background(void)
{
    graphics_clear_screen();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(translation_for(TR_CONFIG_TITLE), 16, 16, 608, FONT_LARGE_BLACK, 0);

    for (int i = 0; i < NUM_VISIBLE_ITEMS; i++) {
        config_widget *w = &widgets[i + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_HEADER) {
            text_draw(translation_for(w->description), 20, y, FONT_NORMAL_BLACK, 0);
        } else if (w->type == TYPE_CHECKBOX) {
            checkbox_draw_text(20, y, w->checkbox_parameter, w->description);
        } else if (w->type == TYPE_SELECT) {
            text_draw(translation_for(w->description), 20, y + 6, FONT_NORMAL_BLACK, 0);
            const generic_button *btn = &select_buttons[w->select_type];
            text_draw_centered(w->get_display_text(), btn->x, y + btn->y + 6, btn->width, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(translation_for(bottom_button_texts[i]),
            bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < NUM_VISIBLE_ITEMS; i++) {
        config_widget *w = &widgets[i + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_CHECKBOX) {
            checkbox_draw(20, y, data.focus_button == i + 1);
        } else if (w->type == TYPE_SELECT) {
            const generic_button *btn = &select_buttons[w->select_type];
            button_border_draw(btn->x, y + btn->y,
                btn->width, btn->height, data.focus_button == i + 1);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y,
            bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }

    inner_panel_draw(scrollbar.x + 4, scrollbar.y + 28, 2, scrollbar.height / 16 - 3);
    scrollbar_draw(&scrollbar);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog)) {
        return;
    }

    int handled = 0;
    data.focus_button = 0;
    
    for (int i = 0; i < NUM_VISIBLE_ITEMS; i++) {
        config_widget *w = &widgets[i + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_CHECKBOX) {
            int focus = 0;
            handled |= checkbox_handle_mouse(m_dialog, 20, y, w->checkbox_parameter, &focus);
            if (focus) {
                data.focus_button = i + 1;
            }
        } else if (w->type == TYPE_SELECT) {
            generic_button *btn = &select_buttons[w->select_type];
            int focus = 0;
            handled |= generic_buttons_handle_mouse(m_dialog, 0, y, btn, 1, &focus);
            if (focus) {
                data.focus_button = i + 1;
            }
        }
    }

    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0,
        bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);

    if (!handled && (m->right.went_up || h->escape_pressed)) {
        window_main_menu_show(0);
    }
}

static void on_scroll(void)
{
    window_invalidate();
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
    const generic_button *btn = &select_buttons[SELECT_LANGUAGE];
    window_select_list_show_text(
        screen_dialog_offset_x() + btn->x + btn->width - 10,
        screen_dialog_offset_y() + 45,
        data.language_options, data.num_language_options, set_language
    );
}

static void set_display_scale(int value)
{
    if (value < 50 || value > 500) {
        return;
    }
    data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].new_value = value;
}

static void button_display_scale_select(int param1, int param2)
{
    const generic_button *btn = &select_buttons[SELECT_DISPLAY];
    window_numeric_input_show(
        screen_dialog_offset_x() + btn->x + btn->width - 10,
        screen_dialog_offset_y() + 45,
        3, 500, set_display_scale
    );
}

static void set_cursor_scale(int value)
{
    switch (value) {
        case 0:
            data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value = 100;
            break;
        case 1:
            data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value = 150;
            break;
        case 2:
            data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value = 200;
            break;
    }
}

static void button_cursor_scale_select(int param1, int param2)
{
    const generic_button *btn = &select_buttons[SELECT_CURSOR];
    static const uint8_t *cursor_options[3];
    cursor_options[0] = string_from_ascii("100%");
    cursor_options[1] = string_from_ascii("150%");
    cursor_options[2] = string_from_ascii("200%");
    window_select_list_show_text(
        screen_dialog_offset_x() + btn->x + btn->width - 10,
        screen_dialog_offset_y() + 45,
        cursor_options, 3, set_cursor_scale
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
        strncpy(data.config_string_values[i].new_value,
            config_get_default_string_value(i), CONFIG_STRING_VALUE_MAX - 1);
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
        // memcpy required to fix warning on Switch build
        memcpy(data.config_string_values[i].new_value,
            data.config_string_values[i].original_value, CONFIG_STRING_VALUE_MAX - 1);
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

void window_config_show(void)
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

static int config_change_display_scale(config_key key)
{
    config_change_basic(key);
    system_scale_display(data.config_values[key].new_value);
    return 1;
}

static int config_change_cursor_scale(config_key key)
{
    config_change_basic(key);
    system_init_cursors(data.config_values[key].new_value);
    return 1;
}

static int config_change_string_basic(config_string_key key)
{
    config_set_string(key, data.config_string_values[key].new_value);
    strncpy(data.config_string_values[key].original_value,
        data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
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
    strncpy(data.config_string_values[key].original_value,
        data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}
