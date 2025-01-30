#include "cck_selection.h"

#include "assets/assets.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "game/file.h"
#include "game/file_io.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "sound/music.h"
#include "widget/minimap.h"
#include "window/mission_briefing.h"

#include <string.h>

#define MAX_SCENARIOS 15

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BACKGROUND_WIDTH 1024
#define BACKGROUND_HEIGHT 768

static void select_scenario(unsigned int index, int is_double_click);
static void button_start_scenario(int param1, int param2);
static void button_back(int param1, int param2);
static void button_toggle_minimap(const generic_button *button);
static void draw_scenario_item(const list_box_item *item);
static void file_tooltip(const list_box_item *item, tooltip_context *c);

static image_button start_button =
{ 600, 440, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, button_start_scenario, button_none, 1, 0, 1 };

static image_button back_button =
{ 330, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_back, button_none, 1, 0, 1 };
static generic_button toggle_minimap_button =
{ 570, 87, 39, 28, button_toggle_minimap };

static list_box_type list_box = {
    .x = 16,
    .y = 210,
    .width_blocks = 18,
    .height_blocks = 16,
    .item_height = 16,
    .draw_inner_panel = 1,
    .extend_to_hidden_scrollbar = 1,
    .decorate_scrollbar = 1,
    .draw_item = draw_scenario_item,
    .on_select = select_scenario,
    .handle_tooltip = file_tooltip
};

static struct {
    unsigned int focus_toggle_button;
    int show_minimap;
    char selected_scenario_filename[FILE_NAME_MAX];
    uint8_t selected_scenario_display[FILE_NAME_MAX];
    saved_game_info info;
    
    const dir_listing *scenarios;
} data;

static void init(void)
{
    data.scenarios = dir_find_files_with_extension_at_location(PATH_LOCATION_SCENARIO, "map");
    data.scenarios = dir_append_files_with_extension("mapx");
    data.focus_toggle_button = 0;
    data.show_minimap = 0;
    data.selected_scenario_display[0] = 0;
    data.selected_scenario_filename[0] = 0;
    list_box_init(&list_box, data.scenarios->num_files);
    list_box_select_index(&list_box, 0);
}

static void draw_scenario_item(const list_box_item *item)
{
    uint8_t displayable_file[FILE_NAME_MAX];
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    encoding_from_utf8(data.scenarios->files[item->index].name, displayable_file, FILE_NAME_MAX);
    file_remove_extension((char *) displayable_file);
    text_ellipsize(displayable_file, font, item->width);
    text_draw(displayable_file, item->x, item->y, font, 0);
    if (item->is_focused) {
        button_border_draw(item->x - 4, item->y - 4, item->width + 6, item->height + 4, 1);
    }
}

static void draw_scenario_info(void)
{
    const int scenario_info_x = 335;
    const int scenario_info_width = 280;
    const int scenario_criteria_x = 420;

    image_draw(image_group(GROUP_SCENARIO_IMAGE) + data.info.image_id, 78, 36, COLOR_MASK_NONE, SCALE_NONE);

    text_ellipsize(data.selected_scenario_display, FONT_LARGE_BLACK, scenario_info_width + 10);
    text_draw_centered(data.selected_scenario_display,
        scenario_info_x, 25, scenario_info_width + 10, FONT_LARGE_BLACK, 0);
    text_draw_centered(data.info.description, scenario_info_x, 60, scenario_info_width, FONT_NORMAL_WHITE, 0);
    lang_text_draw_year(data.info.start_year, scenario_criteria_x, 90, FONT_LARGE_BLACK);

    if (data.show_minimap) {
        widget_minimap_draw(332, 119, 286, 300);
        // minimap button: draw mission instructions image
        image_draw(image_group(GROUP_SIDEBAR_BRIEFING_ROTATE_BUTTONS),
            toggle_minimap_button.x + 3, toggle_minimap_button.y + 3, COLOR_MASK_NONE, SCALE_NONE);
    } else {
        // minimap button: draw minimap
        int minimap_side = toggle_minimap_button.width < toggle_minimap_button.height ?
            toggle_minimap_button.width : toggle_minimap_button.height;
        int minimap_x_offset = (toggle_minimap_button.width - minimap_side) / 2;
        widget_minimap_draw(
            toggle_minimap_button.x + 3 + minimap_x_offset, toggle_minimap_button.y + 3,
            minimap_side - 6, minimap_side - 6
        );

        lang_text_draw_centered(44, 77 + data.info.climate,
            scenario_info_x, 150, scenario_info_width, FONT_NORMAL_BLACK);

        // map size
        int text_id;
        switch (data.info.map_size) {
            case 40: text_id = 121; break;
            case 60: text_id = 122; break;
            case 80: text_id = 123; break;
            case 100: text_id = 124; break;
            case 120: text_id = 125; break;
            default: text_id = 126; break;
        }
        lang_text_draw_centered(44, text_id, scenario_info_x, 170, scenario_info_width, FONT_NORMAL_BLACK);

        // military
        if (data.info.total_invasions <= 0) {
            text_id = 112;
        } else if (data.info.total_invasions <= 2) {
            text_id = 113;
        } else if (data.info.total_invasions <= 4) {
            text_id = 114;
        } else if (data.info.total_invasions <= 10) {
            text_id = 115;
        } else {
            text_id = 116;
        }
        lang_text_draw_centered(44, text_id, scenario_info_x, 190, scenario_info_width, FONT_NORMAL_BLACK);

        lang_text_draw_centered(32, 11 + data.info.player_rank,
            scenario_info_x, 210, scenario_info_width, FONT_NORMAL_BLACK);
        if (data.info.is_open_play) {
            if (data.info.open_play_id < 12) {
                lang_text_draw_multiline(145, data.info.open_play_id,
                    scenario_info_x + 10, 270, scenario_info_width - 10, FONT_NORMAL_BLACK);
            }
        } else {
            lang_text_draw_centered(44, 127, scenario_info_x, 262, scenario_info_width, FONT_NORMAL_BLACK);
            int width;
            if (data.info.win_criteria.culture.enabled) {
                width = text_draw_number(data.info.win_criteria.culture.goal, '@', " ",
                    scenario_criteria_x, 290, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 129, scenario_criteria_x + width, 290, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.prosperity.enabled) {
                width = text_draw_number(data.info.win_criteria.prosperity.goal, '@', " ",
                    scenario_criteria_x, 306, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 130, scenario_criteria_x + width, 306, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.peace.enabled) {
                width = text_draw_number(data.info.win_criteria.peace.goal, '@', " ",
                    scenario_criteria_x, 322, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 131, scenario_criteria_x + width, 322, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.favor.enabled) {
                width = text_draw_number(data.info.win_criteria.favor.goal, '@', " ",
                    scenario_criteria_x, 338, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 132, scenario_criteria_x + width, 338, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.population.enabled) {
                width = text_draw_number(data.info.win_criteria.population.goal, '@', " ",
                    scenario_criteria_x, 354, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 133, scenario_criteria_x + width, 354, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.time_limit.enabled) {
                width = text_draw_number(data.info.win_criteria.time_limit.years, '@', " ",
                    scenario_criteria_x, 370, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 134, scenario_criteria_x + width, 370, FONT_NORMAL_BLACK);
            }
            if (data.info.win_criteria.survival_time.enabled) {
                width = text_draw_number(data.info.win_criteria.survival_time.years, '@', " ",
                    scenario_criteria_x, 386, FONT_NORMAL_BLACK, 0);
                lang_text_draw(44, 135, scenario_criteria_x + width, 386, FONT_NORMAL_BLACK);
            }
        }
    }
    lang_text_draw_centered(44, 136, scenario_info_x, 446, scenario_info_width, FONT_NORMAL_BLACK);
}

static void draw_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_INTERMEZZO_BACKGROUND) + 25);

    graphics_set_clip_rectangle(0, 0,
        WINDOW_WIDTH, WINDOW_HEIGHT);
    graphics_in_dialog();
    image_draw(image_group(GROUP_CCK_BACKGROUND), (WINDOW_WIDTH - BACKGROUND_WIDTH) / 2,
        (WINDOW_HEIGHT - BACKGROUND_HEIGHT) / 2, COLOR_MASK_NONE, SCALE_NONE);
    graphics_reset_clip_rectangle();
    list_box_request_refresh(&list_box);
    draw_scenario_info();
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(0, 0, &start_button, 1);
    image_buttons_draw(0, 0, &back_button, 1);
    button_border_draw(
        toggle_minimap_button.x, toggle_minimap_button.y,
        toggle_minimap_button.width, toggle_minimap_button.height,
        data.focus_toggle_button);
    list_box_draw(&list_box);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    data.focus_toggle_button = 0;
    if (image_buttons_handle_mouse(m_dialog, 0, 0, &start_button, 1, 0) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, &back_button, 1, 0) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, &toggle_minimap_button, 1, &data.focus_toggle_button) ||
        list_box_handle_input(&list_box, m_dialog, 1)) {
        return;
    }
    if (h->enter_pressed) {
        button_start_scenario(0, 0);
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void file_tooltip(const list_box_item *item, tooltip_context *c)
{
    static uint8_t displayable_file[FILE_NAME_MAX];
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    encoding_from_utf8(data.scenarios->files[item->index].name, displayable_file, FILE_NAME_MAX);
    file_remove_extension((char *) displayable_file);
    if (text_get_width(displayable_file, font) > item->width) {
        c->precomposed_text = displayable_file;
        c->type = TOOLTIP_BUTTON;
    }
}

static void handle_tooltip(tooltip_context *c)
{
    list_box_handle_tooltip(&list_box, c);
}

static void button_back(int param1, int param2)
{
    window_go_back();
}

static void select_scenario(unsigned int index, int is_double_click)
{
    if (strcmp(data.selected_scenario_filename, data.scenarios->files[index].name) != 0) {
        snprintf(data.selected_scenario_filename, FILE_NAME_MAX, "%s", data.scenarios->files[index].name);
        const char *filename = dir_get_file_at_location(data.selected_scenario_filename, PATH_LOCATION_SCENARIO);
        if (filename) {
            game_file_io_read_scenario_info(filename, &data.info);
        }
        encoding_from_utf8(data.selected_scenario_filename, data.selected_scenario_display, FILE_NAME_MAX);
        file_remove_extension((char *) data.selected_scenario_display);
        window_invalidate();
    } else if (is_double_click) {
        button_start_scenario(0, 0);
    }
}

static void button_start_scenario(int param1, int param2)
{
    file_remove_extension(data.selected_scenario_filename);
    encoding_from_utf8(data.selected_scenario_filename, data.selected_scenario_display, FILE_NAME_MAX);
    scenario_set_name(data.selected_scenario_display);
    scenario_set_custom(2);
    scenario_set_player_name(lang_get_string(9, 5));
    window_mission_briefing_show();
}

static void button_toggle_minimap(const generic_button *button)
{
    data.show_minimap = !data.show_minimap;
    window_invalidate();
}

void window_cck_selection_show(void)
{
    window_type window = {
        WINDOW_CCK_SELECTION,
        draw_background,
        draw_foreground,
        handle_input,
        handle_tooltip
    };
    init();
    window_show(&window);
}
