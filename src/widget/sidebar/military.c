#include "military.h"

#include "building/count.h"
#include "city/view.h"
#include "core/calc.h"
#include "figure/formation.h"
#include "figure/formation_legion.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "sound/speech.h"
#include "widget/minimap.h"
#include "widget/sidebar/city.h"
#include "widget/sidebar/common.h"
#include "widget/sidebar/extra.h"
#include "widget/sidebar/slide.h"
#include "window/city.h"
#include "window/military_menu.h"

#define LAYOUTS_PER_LEGION 5

#define Y_OFFSET_PANEL_START 176
#define MILITARY_PANEL_BLOCKS 18

static const int IMAGE_OFFSETS_TO_FORMATION[7] = {
    FORMATION_COLUMN,
    FORMATION_TORTOISE,
    FORMATION_DOUBLE_LINE_1,
    FORMATION_DOUBLE_LINE_2,
    FORMATION_MOP_UP,
    FORMATION_SINGLE_LINE_1,
    FORMATION_SINGLE_LINE_2
};

static const int LAYOUT_IMAGE_OFFSETS_LEGIONARY[2][LAYOUTS_PER_LEGION] = {
    {0, 0, 2, 3, 4}, {0, 0, 3, 2, 4},
};

static const int LAYOUT_IMAGE_OFFSETS_AUXILIARY[2][LAYOUTS_PER_LEGION] = {
    {5, 6, 2, 3, 4}, {6, 5, 3, 2, 4},
};

static const int LAYOUT_BUTTON_INDEXES_LEGIONARY[2][LAYOUTS_PER_LEGION] = {
    {
        FORMATION_TORTOISE,
        FORMATION_COLUMN,
        FORMATION_DOUBLE_LINE_1,
        FORMATION_DOUBLE_LINE_2,
        FORMATION_MOP_UP
    },
    {
        FORMATION_TORTOISE,
        FORMATION_COLUMN,
        FORMATION_DOUBLE_LINE_2,
        FORMATION_DOUBLE_LINE_1,
        FORMATION_MOP_UP
    }
};

static const int LAYOUT_BUTTON_INDEXES_AUXILIARY[2][LAYOUTS_PER_LEGION] = {
    {
        FORMATION_SINGLE_LINE_1,
        FORMATION_SINGLE_LINE_2,
        FORMATION_DOUBLE_LINE_1,
        FORMATION_DOUBLE_LINE_2,
        FORMATION_MOP_UP
    },
    {
        FORMATION_SINGLE_LINE_2,
        FORMATION_SINGLE_LINE_1,
        FORMATION_DOUBLE_LINE_2,
        FORMATION_DOUBLE_LINE_1,
        FORMATION_MOP_UP
    }
};

static void button_military_menu(int param1, int param2);
static void button_close_military_sidebar(int param1, int param2);
static void button_cycle_legion(int cycle_forward, int param2);
static void button_select_formation_layout(int index, int param2);
static void button_go_to_legion(int param1, int param2);
static void button_return_to_fort(int param1, int param2);
static void button_empire_service(int param1, int param2);

static image_button buttons_title_close[] = {
    {127, 5, 31, 20, IB_NORMAL, 90, 0, button_close_military_sidebar, button_none, 0, 0, 1},
    {4, 3, 117, 31, IB_NORMAL, 93, 0, button_military_menu, button_none, 0, 0, 1}
};

static arrow_button buttons_cycle_legion[] = {
    {1, 0, 19, 24, button_cycle_legion, 0, 0},
    {125, 0, 21, 24, button_cycle_legion, 1, 0},
};

static generic_button buttons_formation_layout[LAYOUTS_PER_LEGION - 2][LAYOUTS_PER_LEGION] = {
    {
        {2, 0, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {52, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {102, 0, 46, 46, button_select_formation_layout, button_none, 2, 0}
    },
    {
        {27, 50, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {27, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {77, 0, 46, 46, button_select_formation_layout, button_none, 2, 0},
        {77, 50, 46, 46, button_select_formation_layout, button_none, 3, 0}
    },
    {
        {27, 0, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {77, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {2, 50, 46, 46, button_select_formation_layout, button_none, 2, 0},
        {52, 50, 46, 46, button_select_formation_layout, button_none, 3, 0},
        {102, 50, 46, 46, button_select_formation_layout, button_none, 4, 0}
    }
};

static generic_button buttons_bottom[] = {
    {10, 0, 30, 30, button_go_to_legion, button_none, 0, 0},
    {60, 0, 30, 30, button_return_to_fort, button_none, 0, 0},
    {110, 0, 30, 30, button_empire_service, button_none, 0, 0},
};

typedef struct {
    int formation_id;
    int soldiers;
    int health;
    int morale;
    int layout;
    int is_at_fort;
    int empire_service;
} legion_info;

static struct {
    legion_info active_legion;
    int total_selected_legions;
    int top_buttons_focus_id;
    int inner_buttons_focus_id;
    int bottom_buttons_focus_id;
    int city_view_was_collapsed;
} data;

static int available_layouts_for_legion(const formation *m)
{
    int layouts = LAYOUTS_PER_LEGION;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        if (m->has_military_training) {
            layouts = LAYOUTS_PER_LEGION - 1;
        } else {
            layouts = LAYOUTS_PER_LEGION - 2;
        }
    }
    return layouts;
}

static void draw_layout_buttons(int x, int y, int draw_buttons, const formation *m)
{
    int index = 0;
    if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
        index = 1;
    }
    const int *offsets = (m->figure_type == FIGURE_FORT_LEGIONARY) ?
        LAYOUT_IMAGE_OFFSETS_LEGIONARY[index] : LAYOUT_IMAGE_OFFSETS_AUXILIARY[index];
    int formation_types = available_layouts_for_legion(m);

    int start_formation = LAYOUTS_PER_LEGION - formation_types;
    const generic_button *button_offsets = buttons_formation_layout[formation_types - 3];

    for (int i = start_formation; i < LAYOUTS_PER_LEGION; i++) {
        const generic_button *current_button_offset = &button_offsets[i - start_formation];
        int is_selected_formation = m->layout == IMAGE_OFFSETS_TO_FORMATION[offsets[i]];
        int is_button_focused = i == data.inner_buttons_focus_id - 1 + start_formation;
        button_border_draw(x + current_button_offset->x, y + current_button_offset->y, 46, 46,
            is_button_focused || is_selected_formation);

        if (draw_buttons) {
            image_draw_scaled_down(image_group(GROUP_FORT_FORMATIONS) + offsets[i],
                x + current_button_offset->x + 3, y + current_button_offset->y + 3, 2);
        }
    }
}

static int get_health_text_id(int health)
{
    if (health <= 0) {
        return 26;
    } else if (health <= 20) {
        return 27;
    } else if (health <= 40) {
        return 28;
    } else if (health <= 55) {
        return 29;
    } else if (health <= 70) {
        return 30;
    } else if (health <= 90) {
        return 31;
    } else {
        return 32;
    }
}

static void clear_legion_info(legion_info *legion)
{
    legion->health = 0;
    legion->layout = 0;
    legion->morale = 0;
    legion->soldiers = 0;
    legion->is_at_fort = 0;
    legion->empire_service = 0;
}

static void draw_military_info(int x_offset, int y_offset)
{
    legion_info *legion = &data.active_legion;
    const formation *m = formation_get(legion->formation_id);
    int formation_image = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;

    // Legion name
    image_draw(formation_image,
        x_offset + (SIDEBAR_EXPANDED_WIDTH - 12 - image_get(formation_image)->width) / 2, y_offset + 8);
    lang_text_draw_centered(138, m->legion_id, x_offset, y_offset + 34,
        SIDEBAR_EXPANDED_WIDTH - 12, FONT_NORMAL_WHITE);

    // Number of soldiers
    int width = text_draw_number(m->num_figures, '@', " ", x_offset, y_offset + 54, FONT_NORMAL_WHITE);
    lang_text_draw(138, 46 - m->figure_type, x_offset + width, y_offset + 54, FONT_NORMAL_WHITE);

    // No soldiers
    if (!m->num_figures) {
        int group_id, text_id;
        if (m->cursed_by_mars) {
            group_id = 89;
            text_id = 1;
        } else if (building_count_active(BUILDING_BARRACKS)) {
            group_id = 138;
            text_id = 10;
        } else {
            group_id = 138;
            text_id = 11;
        }
        lang_text_draw_multiline(group_id, text_id, x_offset, y_offset + 66,
            SIDEBAR_EXPANDED_WIDTH - 12, FONT_NORMAL_WHITE);
        clear_legion_info(legion);
        return;
    }

    // Morale
    lang_text_draw(138, 36, x_offset, y_offset + 74, FONT_NORMAL_WHITE);
    lang_text_draw(138, 37 + m->morale / 5, x_offset + 4, y_offset + 94,
        m->morale < 13 ? FONT_NORMAL_RED : FONT_NORMAL_GREEN);

    // Health
    int health = calc_percentage(m->total_damage, m->max_total_damage);
    lang_text_draw(138, 24, x_offset, y_offset + 114, FONT_NORMAL_WHITE);
    lang_text_draw(138, get_health_text_id(health), x_offset + 4, y_offset + 134,
        health < 55 ? FONT_NORMAL_GREEN : FONT_NORMAL_RED);

    // Formation layout
    draw_layout_buttons(x_offset, y_offset + 156, 1, m);

    int formation_options_image = image_group(GROUP_FORT_ICONS);

    // Go to legion button
    button_border_draw(x_offset + 10, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 1);
    image_draw(formation_options_image, x_offset + 13, y_offset + 260);

    // Return to fort button
    button_border_draw(x_offset + 60, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 2);
    image_draw(formation_options_image + 1 + m->is_at_fort, x_offset + 63, y_offset + 260);

    // Empire service button
    button_border_draw(x_offset + 110, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 3);
    image_draw(formation_options_image + 4 - m->empire_service, x_offset + 113, y_offset + 260);

    legion->health = health;
    legion->layout = m->layout;
    legion->morale = m->morale;
    legion->soldiers = m->num_figures;
    legion->is_at_fort = m->is_at_fort;
    legion->empire_service = m->empire_service;
}

static void draw_military_panel_background(int x_offset)
{
    graphics_draw_vertical_line(x_offset, Y_OFFSET_PANEL_START,
        Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16, COLOR_WHITE);
    graphics_draw_vertical_line(x_offset + SIDEBAR_EXPANDED_WIDTH - 1,
        Y_OFFSET_PANEL_START, Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16, COLOR_SIDEBAR);
    inner_panel_draw(x_offset + 1, Y_OFFSET_PANEL_START + 10, SIDEBAR_EXPANDED_WIDTH / 16, MILITARY_PANEL_BLOCKS);
    inner_panel_draw(x_offset + 1, Y_OFFSET_PANEL_START, SIDEBAR_EXPANDED_WIDTH / 16, 1);

    draw_military_info(x_offset + 6, Y_OFFSET_PANEL_START);
}

static void update_minimap(void)
{
    widget_minimap_invalidate();
}

static void draw_background(int x_offset)
{
    image_draw(image_group(GROUP_SIDE_PANEL) + 1, x_offset, 24);
    image_buttons_draw(x_offset, 24, buttons_title_close, 2);
    lang_text_draw_centered(61, 5, x_offset, 32, 117, FONT_NORMAL_GREEN);
    widget_minimap_draw(x_offset + 8, 59, 73, 111, 1);
    draw_military_panel_background(x_offset);
    int panel_height = 474; // Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16;
    int extra_height = sidebar_extra_draw_background(x_offset, panel_height,
        SIDEBAR_EXPANDED_WIDTH, sidebar_common_get_height() - panel_height + TOP_MENU_HEIGHT,
        0, SIDEBAR_EXTRA_DISPLAY_ALL);
    sidebar_extra_draw_foreground();

    sidebar_common_draw_relief(x_offset, panel_height + extra_height, GROUP_SIDE_PANEL, 0);
}

void widget_sidebar_military_draw_background(void)
{
    draw_background(sidebar_common_get_x_offset_expanded());
}

static int has_legion_changed(const legion_info *legion, const formation *m)
{
    return legion->health != calc_percentage(m->total_damage, m->max_total_damage) ||
        legion->layout != m->layout ||
        legion->morale != m->morale ||
        legion->soldiers != m->num_figures ||
        legion->is_at_fort != m->is_at_fort ||
        legion->empire_service != m->empire_service;
}

static void draw_military_panel_foreground(void)
{
    int x_offset = sidebar_common_get_x_offset_expanded();
    const formation *m = formation_get(data.active_legion.formation_id);
    if (has_legion_changed(&data.active_legion, m)) {
        draw_military_panel_background(x_offset);
    }
    int num_legions = formation_get_num_legions();
    if (num_legions > 1) {
        arrow_buttons_draw(x_offset + 6, 181, buttons_cycle_legion, 2);
    }
    if (!m->num_figures) {
        return;
    }
    int y_offset = Y_OFFSET_PANEL_START;
    draw_layout_buttons(x_offset + 6, y_offset + 156, 0, m);
    button_border_draw(x_offset + 16, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 1);
    button_border_draw(x_offset + 66, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 2);
    button_border_draw(x_offset + 116, y_offset + 257, 30, 30, data.bottom_buttons_focus_id == 3);
}

void widget_sidebar_military_draw_foreground(void)
{
    int x_offset = sidebar_common_get_x_offset_expanded();
    widget_minimap_draw(x_offset + 8, 59, 73, 111, 1);
    image_buttons_draw(x_offset, 24, buttons_title_close, 2);
    lang_text_draw_centered(61, 5, x_offset, 32, 117, FONT_NORMAL_GREEN);
    draw_military_panel_foreground();
    sidebar_extra_draw_foreground();
}

int widget_sidebar_military_handle_input(const mouse *m)
{
    int x_offset = sidebar_common_get_x_offset_expanded();
    if (image_buttons_handle_mouse(m, x_offset, 24, buttons_title_close, 2, &data.top_buttons_focus_id)) {
        return 1;
    }
    int num_legions = formation_get_num_legions();
    if (num_legions > 1 &&
        arrow_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + 6, buttons_cycle_legion, 2, 0)) {
        return 1;
    }
    const formation *selected_legion = formation_get(data.active_legion.formation_id);
    if (selected_legion->num_figures > 0) {
        generic_button *layout_buttons = buttons_formation_layout[available_layouts_for_legion(selected_legion) - 3];
        if (generic_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + 156,
            layout_buttons, 5, &data.inner_buttons_focus_id)) {
            return 1;
        }
        if (generic_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + 257,
            buttons_bottom, 3, &data.bottom_buttons_focus_id)) {
            return 1;
        }
    }
    return sidebar_extra_handle_mouse(m);
}

static int get_layout_text_id(int layout)
{
    switch (layout) {
        case FORMATION_SINGLE_LINE_1:
        case FORMATION_SINGLE_LINE_2:
            return 16;
        case FORMATION_DOUBLE_LINE_1:
        case FORMATION_DOUBLE_LINE_2:
            return 14;
        case FORMATION_TORTOISE:
            return 12;
        case FORMATION_MOP_UP:
            return 15;
        case FORMATION_COLUMN:
            return 13;
        default:
            return 16;
    }
}

int widget_sidebar_military_get_tooltip_text(tooltip_context *c)
{
    if (data.top_buttons_focus_id) {
        if (data.top_buttons_focus_id == 1) {
            c->text_group = 68;
            return 2;
        }
        return 0;
    }
    if (data.inner_buttons_focus_id) {
        int index = data.inner_buttons_focus_id - 1;
        int layout;
        const formation *m = formation_get(data.active_legion.formation_id);
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            int index_increase = m->has_military_training ? 1 : 2;
            if (index > 4 - index_increase) {
                return 0;
            }
            index += index_increase;
            layout = LAYOUT_BUTTON_INDEXES_LEGIONARY[0][index];
        } else {
            layout = LAYOUT_BUTTON_INDEXES_AUXILIARY[0][index];
        }
        c->text_group = 138;
        return get_layout_text_id(layout);
    }
    if (data.bottom_buttons_focus_id) {
        c->extra_text_type = TOOLTIP_EXTRA_TEXT_JOINED_BY_SPACE;
        c->num_extra_texts = 1;
        c->text_group = 51;
        c->extra_text_groups[0] = 51;
        int text_id = data.bottom_buttons_focus_id * 2;
        c->extra_text_ids[0] = text_id;
        return text_id - 1;
    }
    return 0;
}

static void set_formation_id(int formation_id)
{
    data.active_legion.formation_id = formation_id;
    clear_legion_info(&data.active_legion);
    update_minimap();
}

static void slide_in_finished(void)
{
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
    }
    update_minimap();
    window_city_return();
}

static void slide_out_finished(void)
{
    update_minimap();
    window_city_show();
}

int widget_sidebar_military_enter(int formation_id)
{
    int had_selected_legions = data.active_legion.formation_id;
    set_formation_id(formation_id);
    if (had_selected_legions) {
        return 0;
    }
    data.city_view_was_collapsed = city_view_is_sidebar_collapsed();
    if (data.city_view_was_collapsed) {
        city_view_start_sidebar_toggle();
        sidebar_slide(SLIDE_DIRECTION_IN, widget_sidebar_city_draw_background, draw_background, slide_in_finished);
    } else {
        slide_in_finished();
    }
    return 1;
}

int widget_sidebar_military_exit(void)
{
    data.active_legion.formation_id = 0;
    if (!window_is(WINDOW_CITY_MILITARY)) {
        update_minimap();
        return 0;
    }
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
        sidebar_slide(SLIDE_DIRECTION_OUT, widget_sidebar_city_draw_background, draw_background, slide_out_finished);
    } else {
        slide_out_finished();
    }
    return 1;
}

static void button_military_menu(int param1, int param2)
{
    window_military_menu_show();
}

static void button_close_military_sidebar(int param1, int param2)
{
    widget_sidebar_military_exit();
}

static void button_cycle_legion(int cycle_forward, int param2)
{
    legion_info *legion = &data.active_legion;
    int step = cycle_forward ? 1 : -1;
    const formation *m;
    for (int i = legion->formation_id + step; i != legion->formation_id; i += step) {
        if (i == 0) {
            i = MAX_FORMATIONS;
        } else if (i > MAX_FORMATIONS) {
            i = 1;
        }
        m = formation_get(i);
        if (m->in_use && !m->is_herd && m->is_legion) {
            legion->formation_id = i;
            break;
        }
    }
    formation_set_selected(legion->formation_id);
    set_formation_id(legion->formation_id);
}

static void button_select_formation_layout(int index, int param2)
{
    formation *m = formation_get(data.active_legion.formation_id);
    if (m->in_distant_battle) {
        return;
    }
    const int *layout_indexes;
    int swap_lines = city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        int index_increase = m->has_military_training ? 1 : 2;
        if (index > 4 - index_increase) {
            return;
        }
        index += index_increase;
        layout_indexes = LAYOUT_BUTTON_INDEXES_LEGIONARY[swap_lines];
    } else {
        layout_indexes = LAYOUT_BUTTON_INDEXES_AUXILIARY[swap_lines];
    }
    formation_legion_change_layout(m, layout_indexes[index]);
    switch (index) {
        case 0: sound_speech_play_file("wavs/cohort1.wav"); break;
        case 1: sound_speech_play_file("wavs/cohort2.wav"); break;
        case 2: sound_speech_play_file("wavs/cohort3.wav"); break;
        case 3: sound_speech_play_file("wavs/cohort4.wav"); break;
        case 4: sound_speech_play_file("wavs/cohort5.wav"); break;
    }
}

static void button_go_to_legion(int param1, int param2)
{
    const formation *m = formation_get(data.active_legion.formation_id);
    city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
}

static void button_return_to_fort(int param1, int param2)
{
    formation *m = formation_get(data.active_legion.formation_id);
    if (!m->in_distant_battle) {
        formation_legion_return_home(m);
    }
}

static void button_empire_service(int param1, int param2)
{
    formation_toggle_empire_service(data.active_legion.formation_id);
    formation_calculate_figures();
}
