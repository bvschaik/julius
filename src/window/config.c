#include "config.h"

#include "core/config.h"
#include "core/dir.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
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
#include "window/plain_message_dialog.h"
#include "window/select_list.h"
#include "translation/translation.h"
#include <string.h>

#define NUM_CHECKBOXES 37
#define CONFIG_PAGES 3
#define MAX_LANGUAGE_DIRS 20

#define FIRST_BUTTON_Y 72
#define BUTTON_SPACING 24
#define TEXT_Y_OFFSET 4

#define CHECKBOX_CHECK_SIZE 20
#define CHECKBOX_HEIGHT 20
#define CHECKBOX_WIDTH 560

#define NUM_VISIBLE_ITEMS 15

#define ITEM_Y_OFFSET 60
#define ITEM_HEIGHT 24

static int options_per_page[CONFIG_PAGES] = { 11,13,12 };

static void toggle_switch(int id, int param2);
static void button_language_select(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_hotkeys(int param1, int param2);
static void button_close(int save, int param2);
static void button_page(int param1, int param2);
static int config_change_basic(config_key key);
static int config_change_zoom(config_key key);
static int config_change_string_basic(config_string_key key);
static int config_change_string_language(config_string_key key);


static generic_button checkbox_buttons[] = {
    { 20,  72, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_INTRO_VIDEO, TR_CONFIG_SHOW_INTRO_VIDEO },
    { 20,  96, 20, 20, toggle_switch, button_none, CONFIG_UI_SIDEBAR_INFO, TR_CONFIG_SIDEBAR_INFO },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_UI_SMOOTH_SCROLLING, TR_CONFIG_SMOOTH_SCROLLING },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_UI_WALKER_WAYPOINTS, TR_CONFIG_DRAW_WALKER_WAYPOINTS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE, TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE, TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_UI_SHOW_CONSTRUCTION_SIZE, TR_CONFIG_SHOW_CONSTRUCTION_SIZE },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_UI_ZOOM, TR_CONFIG_ENABLE_ZOOM },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_UI_COMPLETE_RATING_COLUMNS, TR_CONFIG_COMPLETE_RATING_COLUMNS },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_UI_HIGHLIGHT_LEGIONS, TR_CONFIG_HIGHLIGHT_LEGIONS  },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_UI_ROTATE_MANUALLY, TR_CONFIG_ROTATE_MANUALLY  },
    { 20,  72, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_IMMIGRATION_BUG, TR_CONFIG_FIX_IMMIGRATION_BUG },
    { 20,  96, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_100_YEAR_GHOSTS, TR_CONFIG_FIX_100_YEAR_GHOSTS },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GRANDFESTIVAL, TR_CONFIG_GRANDFESTIVAL },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_JEALOUS_GODS, TR_CONFIG_JEALOUS_GODS },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GLOBAL_LABOUR, TR_CONFIG_GLOBAL_LABOUR },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_SCHOOL_WALKERS, TR_CONFIG_SCHOOL_WALKERS },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_RETIRE_AT_60, TR_CONFIG_RETIRE_AT_60 },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_FIXED_WORKERS, TR_CONFIG_FIXED_WORKERS },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WOLVES_BLOCK, TR_CONFIG_WOLVES_BLOCK },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_DYNAMIC_GRANARIES, TR_CONFIG_DYNAMIC_GRANARIES },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_MORE_STOCKPILE, TR_CONFIG_MORE_STOCKPILE },
    { 20, 336, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_NO_BUYER_DISTRIBUTION, TR_CONFIG_NO_BUYER_DISTRIBUTION },
    { 20, 360, 20, 20, toggle_switch, button_none, CONFIG_GP_FIX_EDITOR_EVENTS, TR_CONFIG_FIX_EDITOR_EVENTS },
    { 20,  72, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_IMMEDIATELY_DELETE_BUILDINGS, TR_CONFIG_IMMEDIATELY_DELETE_BUILDINGS },
    { 20,  96, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD, TR_CONFIG_GETTING_GRANARIES_GO_OFFROAD },
    { 20, 120, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_GRANARIES_GET_DOUBLE, TR_CONFIG_GRANARIES_GET_DOUBLE },
    { 20, 144, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD, TR_CONFIG_TOWER_SENTRIES_GO_OFFROAD },
    { 20, 168, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_FARMS_DELIVER_CLOSE, TR_CONFIG_FARMS_DELIVER_CLOSE },
    { 20, 192, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_DELIVER_ONLY_TO_ACCEPTING_GRANARIES, TR_CONFIG_DELIVER_ONLY_TO_ACCEPTING_GRANARIES },
    { 20, 216, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_ALL_HOUSES_MERGE, TR_CONFIG_ALL_HOUSES_MERGE },
    { 20, 240, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WINE_COUNTS_IF_OPEN_TRADE_ROUTE, TR_CONFIG_WINE_COUNTS_IF_OPEN_TRADE_ROUTE },
    { 20, 264, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY, TR_CONFIG_RANDOM_COLLAPSES_TAKE_MONEY },
    { 20, 288, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_MULTIPLE_BARRACKS, TR_CONFIG_MULTIPLE_BARRACKS },
    { 20, 312, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT, TR_CONFIG_NOT_ACCEPTING_WAREHOUSES },
    { 20, 336, 20, 20, toggle_switch, button_none, CONFIG_GP_CH_HOUSES_DONT_EXPAND_INTO_GARDENS, TR_CONFIG_HOUSES_DONT_EXPAND_INTO_GARDENS },
};

static generic_button language_button = {
    120, 50, 200, 24, button_language_select, button_none, 0, TR_CONFIG_LANGUAGE_LABEL
};

static generic_button bottom_buttons[] = {
    { 250, 436, 150, 30, button_reset_defaults, button_none, 0, TR_BUTTON_RESET_DEFAULTS },
    { 410, 436, 100, 30, button_close, button_none, 0, TR_BUTTON_CANCEL },
    { 520, 436, 100, 30, button_close, button_none, 1, TR_BUTTON_OK },
    { 20, 436, 180, 30, button_hotkeys, button_none, 0, TR_BUTTON_CONFIGURE_HOTKEYS }
};

static generic_button page_buttons[] = {
    { 20, 410, 25, 25, button_page, button_none, 0, TR_BUTTON_PREV },
    { 160, 410, 25, 25, button_page, button_none, 1, TR_BUTTON_NEXT }
};

static translation_key page_names[] = {
    TR_CONFIG_HEADER_UI_CHANGES,
    TR_CONFIG_HEADER_GAMEPLAY_CHANGES,
    TR_CONFIG_HEADER_GAMEPLAY_CHANGES
};

static struct {
    int focus_button;
    int language_focus_button;
    int bottom_focus_button;
    int page_focus_button;
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
    data.config_values[CONFIG_UI_ZOOM].change_action = config_change_zoom;
    data.config_string_values[0].change_action = config_change_string_language;

    string_copy(translation_for(TR_CONFIG_LANGUAGE_DEFAULT), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
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
    graphics_clear_screens();

    image_draw_fullscreen_background(image_group(GROUP_CONFIG));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(translation_for(page_names[data.page]), 16, 16, 608, FONT_LARGE_BLACK, 0);

    text_draw(translation_for(TR_CONFIG_LANGUAGE_LABEL), 20, 56, FONT_NORMAL_BLACK, 0);
    text_draw_centered(data.language_options[data.selected_language_option],
        language_button.x, language_button.y + 6, language_button.width, FONT_NORMAL_BLACK, 0);

    for (int i = 0; i < options_per_page[data.page]; i++) {
        text_draw(translation_for(checkbox_buttons[data.starting_option + i].parameter2), 44, FIRST_BUTTON_Y + BUTTON_SPACING * i + TEXT_Y_OFFSET, FONT_NORMAL_BLACK, 0);
    }
    for (int i = 0; i < options_per_page[data.page]; i++) {
        int value = i + data.starting_option;
        generic_button* btn = &checkbox_buttons[value];
        if (data.config_values[btn->parameter1].new_value) {
            text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < sizeof(bottom_buttons) / sizeof(*bottom_buttons); i++) {
        text_draw_centered(translation_for(bottom_buttons[i].parameter2), bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }
    for (int i = 0; i < sizeof(page_buttons) / sizeof(*page_buttons); i++) {
        text_draw_centered(translation_for(page_buttons[i].parameter2), page_buttons[i].x, page_buttons[i].y + 6, page_buttons[i].width, FONT_NORMAL_BLACK, 0);
    }

    text_draw_label_and_number_centered(translation_for(TR_CONFIG_PAGE_LABEL), data.page + 1, "", 60, 416, 85, FONT_NORMAL_BLACK, 0);

    //text_draw_centered(translation_for(TR_CONFIG_PAGE_LABEL), 80, 415, 30, FONT_NORMAL_BLACK, 0);
    //text_draw_number(data.page + 1, '@', " ", 120, 415, FONT_NORMAL_BLACK);

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
    for (int i = 0; i < sizeof(bottom_buttons) / sizeof(*bottom_buttons); i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }
    for (int i = 0; i < sizeof(page_buttons) / sizeof(*page_buttons); i++) {
        button_border_draw(page_buttons[i].x, page_buttons[i].y, page_buttons[i].width, page_buttons[i].height, data.page_focus_button == i + 1);
    }
    button_border_draw(language_button.x, language_button.y, language_button.width, language_button.height, data.language_focus_button == 1);
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    int handled = 0;
    handled |= generic_buttons_min_handle_mouse(m_dialog, 0, 0, checkbox_buttons, data.starting_option + options_per_page[data.page], &data.focus_button, data.starting_option);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, sizeof(bottom_buttons) / sizeof(*bottom_buttons), &data.bottom_focus_button);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, page_buttons, sizeof(page_buttons) / sizeof(*page_buttons), &data.page_focus_button);
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
        screen_dialog_offset_y() + 45,
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
        memcpy(data.config_string_values[i].new_value, data.config_string_values[i].original_value, CONFIG_STRING_VALUE_MAX - 1); // memcpy required to fix warning on Switch build
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

static int config_change_string_basic(config_string_key key)
{
    config_set_string(key, data.config_string_values[key].new_value);
    strncpy(data.config_string_values[key].original_value, data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}

static int config_change_zoom(config_key key)
{
    config_change_basic(key);
    system_reload_textures();
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
