#include "cck_selection.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/string.h"
#include "game/file.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/keyboard.h"
#include "scenario/criteria.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "window/city.h"

#include <string.h>

static void button_select_item(int index, int param2);
static void button_scroll(int is_down, int num_lines);
static void button_start_scenario(int param1, int param2);

static image_button image_buttons[] = {
    {276, 210, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 8, button_scroll, button_none, 0, 1, 1},
    {276, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 12, button_scroll, button_none, 1, 1, 1},
    {600, 440, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, button_start_scenario, button_none, 1, 0, 1},
};

static generic_button file_buttons[] = {
    {18, 220, 252, 16, button_select_item, button_none, 0, 0},
    {18, 236, 252, 16, button_select_item, button_none, 1, 0},
    {18, 252, 252, 16, button_select_item, button_none, 2, 0},
    {18, 268, 252, 16, button_select_item, button_none, 3, 0},
    {18, 284, 252, 16, button_select_item, button_none, 4, 0},
    {18, 300, 252, 16, button_select_item, button_none, 5, 0},
    {18, 316, 252, 16, button_select_item, button_none, 6, 0},
    {18, 332, 252, 16, button_select_item, button_none, 7, 0},
    {18, 348, 252, 16, button_select_item, button_none, 8, 0},
    {18, 364, 252, 16, button_select_item, button_none, 9, 0},
    {18, 380, 252, 16, button_select_item, button_none, 10, 0},
    {18, 396, 252, 16, button_select_item, button_none, 11, 0},
    {18, 412, 252, 16, button_select_item, button_none, 12, 0},
    {18, 428, 252, 16, button_select_item, button_none, 13, 0},
    {18, 444, 252, 16, button_select_item, button_none, 14, 0},
};

static struct {
    int scroll_position;
    int focus_button_id;
    int selected_item;
    char selected_scenario_filename[FILE_NAME_MAX];
    uint8_t selected_scenario_display[FILE_NAME_MAX];

    const dir_listing *scenarios;
} data;

static void init(void)
{
    scenario_set_custom(2);
    data.scenarios = dir_find_files_with_extension("map");
    data.scroll_position = 0;
    data.focus_button_id = 0;
    button_select_item(0, 0);
}

static void draw_scenario_list(void)
{
    inner_panel_draw(16, 210, 16, 16);
    char file[FILE_NAME_MAX];
    uint8_t displayable_file[FILE_NAME_MAX];
    for (int i = 0; i < 15; i++) {
        font_t font = FONT_NORMAL_GREEN;
        if (data.focus_button_id == i + 1) {
            font = FONT_NORMAL_WHITE;
        } else if (!data.focus_button_id && data.selected_item == i + data.scroll_position) {
            font = FONT_NORMAL_WHITE;
        }
        strcpy(file, data.scenarios->files[i + data.scroll_position]);
        encoding_from_utf8(file, displayable_file, FILE_NAME_MAX);
        file_remove_extension(displayable_file);
        text_ellipsize(displayable_file, font, 240);
        text_draw(displayable_file, 24, 220 + 16 * i, font, 0);
    }
}

static void draw_scrollbar_dot(void)
{
    if (data.scenarios->num_files > 15) {
        int pct;
        if (data.scroll_position <= 0) {
            pct = 0;
        } else if (data.scroll_position + 15 >= data.scenarios->num_files) {
            pct = 100;
        } else {
            pct = calc_percentage(data.scroll_position, data.scenarios->num_files - 15);
        }
        int y_offset = calc_adjust_with_percentage(164, pct);
        image_draw(image_group(GROUP_PANEL_BUTTON) + 39, 284, 245 + y_offset);
    }
}

static void draw_scenario_info(void)
{
    const int scenario_info_x = 335;
    const int scenario_info_width = 280;
    const int scenario_criteria_x = 420;

    image_draw(image_group(GROUP_SCENARIO_IMAGE) + scenario_image_id(), 78, 36);

    text_ellipsize(data.selected_scenario_display, FONT_LARGE_BLACK, scenario_info_width + 10);
    text_draw_centered(data.selected_scenario_display, scenario_info_x, 25, scenario_info_width + 10, FONT_LARGE_BLACK, 0);
    text_draw_centered(scenario_brief_description(), scenario_info_x, 60, scenario_info_width, FONT_NORMAL_WHITE, 0);
    lang_text_draw_year(scenario_property_start_year(), scenario_criteria_x, 90, FONT_LARGE_BLACK);
    lang_text_draw_centered(44, 77 + scenario_property_climate(), scenario_info_x, 150, scenario_info_width, FONT_NORMAL_BLACK);

    // map size
    int text_id;
    switch (scenario_map_size()) {
        case 40: text_id = 121; break;
        case 60: text_id = 122; break;
        case 80: text_id = 123; break;
        case 100: text_id = 124; break;
        case 120: text_id = 125; break;
        default: text_id = 126; break;
    }
    lang_text_draw_centered(44, text_id, scenario_info_x, 170, scenario_info_width, FONT_NORMAL_BLACK);

    // military
    int num_invasions = scenario_invasion_count();
    if (num_invasions <= 0) {
        text_id = 112;
    } else if (num_invasions <= 2) {
        text_id = 113;
    } else if (num_invasions <= 4) {
        text_id = 114;
    } else if (num_invasions <= 10) {
        text_id = 115;
    } else {
        text_id = 116;
    }
    lang_text_draw_centered(44, text_id, scenario_info_x, 190, scenario_info_width, FONT_NORMAL_BLACK);

    lang_text_draw_centered(32, 11 + scenario_property_player_rank(), scenario_info_x, 210, scenario_info_width, FONT_NORMAL_BLACK);
    if (scenario_is_open_play()) {
        if (scenario_open_play_id() < 12) {
            lang_text_draw_multiline(145, scenario_open_play_id(), scenario_info_x + 10, 270, scenario_info_width - 10, FONT_NORMAL_BLACK);
        }
    } else {
        lang_text_draw_centered(44, 127, scenario_info_x, 262, scenario_info_width, FONT_NORMAL_BLACK);
        int width;
        if (scenario_criteria_culture_enabled()) {
            width = text_draw_number(scenario_criteria_culture(), '@', " ", scenario_criteria_x, 290, FONT_NORMAL_BLACK);
            lang_text_draw(44, 129, scenario_criteria_x + width, 290, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_prosperity_enabled()) {
            width = text_draw_number(scenario_criteria_prosperity(), '@', " ", scenario_criteria_x, 306, FONT_NORMAL_BLACK);
            lang_text_draw(44, 130, scenario_criteria_x + width, 306, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_peace_enabled()) {
            width = text_draw_number(scenario_criteria_peace(), '@', " ", scenario_criteria_x, 322, FONT_NORMAL_BLACK);
            lang_text_draw(44, 131, scenario_criteria_x + width, 322, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_favor_enabled()) {
            width = text_draw_number(scenario_criteria_favor(), '@', " ", scenario_criteria_x, 338, FONT_NORMAL_BLACK);
            lang_text_draw(44, 132, scenario_criteria_x + width, 338, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_population_enabled()) {
            width = text_draw_number(scenario_criteria_population(), '@', " ", scenario_criteria_x, 354, FONT_NORMAL_BLACK);
            lang_text_draw(44, 133, scenario_criteria_x + width, 354, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_time_limit_enabled()) {
            width = text_draw_number(scenario_criteria_time_limit_years(), '@', " ", scenario_criteria_x, 370, FONT_NORMAL_BLACK);
            lang_text_draw(44, 134, scenario_criteria_x + width, 370, FONT_NORMAL_BLACK);
        }
        if (scenario_criteria_survival_enabled()) {
            width = text_draw_number(scenario_criteria_survival_years(), '@', " ", scenario_criteria_x, 386, FONT_NORMAL_BLACK);
            lang_text_draw(44, 135, scenario_criteria_x + width, 386, FONT_NORMAL_BLACK);
        }
    }
    lang_text_draw_centered(44, 136, scenario_info_x, 446, scenario_info_width, FONT_NORMAL_BLACK);
}

static void draw_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_CCK_BACKGROUND));
    graphics_in_dialog();
    inner_panel_draw(280, 242, 2, 12);
    draw_scenario_list();
    draw_scrollbar_dot();
    draw_scenario_info();
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(0, 0, image_buttons, 3);
    draw_scenario_list();
    graphics_reset_dialog();
}

static int handle_scrollbar(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (data.scenarios->num_files <= 15) {
        return 0;
    }
    if (!m_dialog->left.is_down) {
        return 0;
    }
    if (m_dialog->x >= 280 && m_dialog->x <= 312 &&
            m_dialog->y >= 245 && m_dialog->y <= 434) {
        int y_offset = m_dialog->y - 245;
        if (y_offset > 164) {
            y_offset = 164;
        }
        int pct = calc_percentage(y_offset, 164);
        data.scroll_position = calc_adjust_with_percentage(data.scenarios->num_files - 15, pct);
        window_invalidate();
        return 1;
    }
    return 0;
}

static void handle_mouse(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN) {
        button_scroll(1, 3);
    } else if (m->scrolled == SCROLL_UP) {
        button_scroll(0, 3);
    }
    if (handle_scrollbar(m)) {
        return;
    }
    const mouse *m_dialog = mouse_in_dialog(m);
    if (image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 3, 0)) {
        return;
    }
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, file_buttons, 15, &data.focus_button_id)) {
        return;
    }
    if (keyboard_input_is_accepted()) {
        button_start_scenario(0, 0);
        return;
    }
    if (m->right.went_up || (m->is_touch && m->left.double_click)) {
        window_go_back();
    }
}

static void button_select_item(int index, int param2)
{
    if (index >= data.scenarios->num_files) {
        return;
    }
    data.selected_item = data.scroll_position + index;
    strcpy(data.selected_scenario_filename, data.scenarios->files[data.selected_item]);
    game_file_load_scenario_data(data.selected_scenario_filename);
    encoding_from_utf8(data.selected_scenario_filename, data.selected_scenario_display, FILE_NAME_MAX);
    file_remove_extension(data.selected_scenario_display);
    window_invalidate();
}

static void button_scroll(int is_down, int num_lines)
{
    if (data.scenarios->num_files > 15) {
        if (is_down) {
            data.scroll_position += num_lines;
            if (data.scroll_position > data.scenarios->num_files - 15) {
                data.scroll_position = data.scenarios->num_files - 15;
            }
        } else {
            data.scroll_position -= num_lines;
            if (data.scroll_position < 0) {
                data.scroll_position = 0;
            }
        }
        window_invalidate();
    }
}

static void button_start_scenario(int param1, int param2)
{
    if (game_file_start_scenario(data.selected_scenario_filename)) {
        sound_music_update(1);
        window_city_show();
    }
}

void window_cck_selection_show(void)
{
    window_type window = {
        WINDOW_CCK_SELECTION,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    init();
    window_show(&window);
}
