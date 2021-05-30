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

#define MILITARY_PANEL_HEIGHT 474
#define Y_OFFSET_PANEL_START 176
#define Y_OFFSET_LAYOUT_BUTTONS 156
#define Y_OFFSET_BOTTOM_BUTTONS 257
#define MILITARY_PANEL_BLOCKS 18
#define CONTENT_PADDING 10
#define CONTENT_WIDTH (SIDEBAR_EXPANDED_WIDTH - 2 * CONTENT_PADDING)

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
    {10, 10, 19, 24, button_cycle_legion, 0, 0},
    {126, 10, 21, 24, button_cycle_legion, 1, 0},
};

static generic_button buttons_formation_layout[LAYOUTS_PER_LEGION - 2][LAYOUTS_PER_LEGION] = {
    {
        {8, 0, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {58, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {108, 0, 46, 46, button_select_formation_layout, button_none, 2, 0}
    },
    {
        {33, 50, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {33, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {83, 0, 46, 46, button_select_formation_layout, button_none, 2, 0},
        {83, 50, 46, 46, button_select_formation_layout, button_none, 3, 0}
    },
    {
        {33, 0, 46, 46, button_select_formation_layout, button_none, 0, 0},
        {83, 0, 46, 46, button_select_formation_layout, button_none, 1, 0},
        {8, 50, 46, 46, button_select_formation_layout, button_none, 2, 0},
        {58, 50, 46, 46, button_select_formation_layout, button_none, 3, 0},
        {108, 50, 46, 46, button_select_formation_layout, button_none, 4, 0}
    }
};

static generic_button buttons_bottom[] = {
    {16, 0, 30, 30, button_go_to_legion, button_none, 0, 0},
    {66, 0, 30, 30, button_return_to_fort, button_none, 0, 0},
    {116, 0, 30, 30, button_empire_service, button_none, 0, 0},
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

static void draw_layout_buttons(int x, int y, int background, const formation *m)
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
        const generic_button *btn = &button_offsets[i - start_formation];

        if (background) {
            image_draw_scaled_down(image_group(GROUP_FORT_FORMATIONS) + offsets[i],
                x + btn->x + 3, y + btn->y + 3, 2);
        } else {
            int is_selected_formation = m->layout == IMAGE_OFFSETS_TO_FORMATION[offsets[i]];
            int is_button_focused = i == data.inner_buttons_focus_id - 1 + start_formation;
            button_border_draw(x + btn->x, y + btn->y, 46, 46, is_button_focused || is_selected_formation);
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

static void clear_focus_buttons(void)
{
    data.top_buttons_focus_id = 0;
    data.inner_buttons_focus_id = 0;
    data.bottom_buttons_focus_id = 0;
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

static void update_legion_info(legion_info *legion, const formation *m)
{
    legion->health = calc_percentage(m->total_damage, m->max_total_damage);
    legion->layout = m->layout;
    legion->morale = m->morale;
    legion->soldiers = m->num_figures;
    legion->is_at_fort = m->is_at_fort;
    legion->empire_service = m->empire_service;
}

static void draw_military_info_text(int x_offset, int y_offset)
{
    legion_info *legion = &data.active_legion;
    const formation *m = formation_get(legion->formation_id);
    update_legion_info(legion, m);

    int formation_image = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;

    // Legion name
    image_draw(formation_image,
        x_offset + (CONTENT_WIDTH - image_get(formation_image)->width) / 2, y_offset + 12);
    lang_text_draw_centered(138, m->legion_id, x_offset, y_offset + 40, CONTENT_WIDTH, FONT_NORMAL_WHITE);

    // Number of soldiers
    int width = text_draw_number(m->num_figures, '@', " ", x_offset, y_offset + 60, FONT_NORMAL_WHITE);
    lang_text_draw(138, 46 - m->figure_type, x_offset + width, y_offset + 60, FONT_NORMAL_WHITE);

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
        lang_text_draw_multiline(group_id, text_id, x_offset, y_offset + 80, CONTENT_WIDTH, FONT_NORMAL_WHITE);
        clear_legion_info(legion);
        return;
    }

    int ellipsized_width = CONTENT_WIDTH + CONTENT_PADDING / 2;
    // Morale
    lang_text_draw_ellipsized(138, 36, x_offset, y_offset + 80, ellipsized_width, FONT_NORMAL_WHITE);
    lang_text_draw_ellipsized(138, 37 + m->morale / 5, x_offset + 4, y_offset + 98,
        ellipsized_width, m->morale < 13 ? FONT_NORMAL_RED : FONT_NORMAL_GREEN);

    // Health
    lang_text_draw_ellipsized(138, 24, x_offset, y_offset + 120, ellipsized_width, FONT_NORMAL_WHITE);
    lang_text_draw_ellipsized(138, get_health_text_id(legion->health), x_offset + 4, y_offset + 138,
        ellipsized_width, legion->health < 55 ? FONT_NORMAL_GREEN : FONT_NORMAL_RED);
}

static void draw_military_info_buttons(int x_offset, int y_offset)
{
    if (!data.active_legion.soldiers) {
        return;
    }
    const formation *m = formation_get(data.active_legion.formation_id);
    // Formation layout
    draw_layout_buttons(x_offset, y_offset + Y_OFFSET_LAYOUT_BUTTONS, 1, m);

    int formation_options_image = image_group(GROUP_FORT_ICONS);

    // Go to legion button
    const generic_button *btn = buttons_bottom;
    image_draw(formation_options_image, x_offset + btn->x + 3, y_offset + 260);

    // Return to fort button
    ++btn;
    image_draw(formation_options_image + 1 + m->is_at_fort, x_offset + btn->x + 3, y_offset + 260);

    // Empire service button
    ++btn;
    image_draw(formation_options_image + 4 - m->empire_service, x_offset + btn->x + 3, y_offset + 260);
}

static void draw_military_panel_background(int x_offset)
{
    graphics_draw_vertical_line(x_offset, Y_OFFSET_PANEL_START,
        Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * BLOCK_SIZE, COLOR_WHITE);
    graphics_draw_vertical_line(x_offset + SIDEBAR_EXPANDED_WIDTH - 1,
        Y_OFFSET_PANEL_START, Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * BLOCK_SIZE, COLOR_SIDEBAR);
    inner_panel_draw(x_offset + 1, Y_OFFSET_PANEL_START + 10,
        SIDEBAR_EXPANDED_WIDTH / BLOCK_SIZE, MILITARY_PANEL_BLOCKS);
    inner_panel_draw(x_offset + 1, Y_OFFSET_PANEL_START, SIDEBAR_EXPANDED_WIDTH / BLOCK_SIZE, 1);

    draw_military_info_text(x_offset + CONTENT_PADDING, Y_OFFSET_PANEL_START);
    draw_military_info_buttons(x_offset, Y_OFFSET_PANEL_START);
}

static void draw_background(int x_offset)
{
    image_draw(image_group(GROUP_SIDE_PANEL) + 1, x_offset, 24);
    image_buttons_draw(x_offset, 24, buttons_title_close, 2);
    lang_text_draw_centered(61, 5, x_offset, 32, 117, FONT_NORMAL_GREEN);
    widget_minimap_draw(x_offset + 8, 59, MINIMAP_WIDTH, MINIMAP_HEIGHT, 1);
    draw_military_panel_background(x_offset);
    int extra_height = sidebar_extra_draw_background(x_offset, MILITARY_PANEL_HEIGHT,
        SIDEBAR_EXPANDED_WIDTH, sidebar_common_get_height() - MILITARY_PANEL_HEIGHT + TOP_MENU_HEIGHT,
        0, SIDEBAR_EXTRA_DISPLAY_ALL);

    sidebar_common_draw_relief(x_offset, MILITARY_PANEL_HEIGHT + extra_height, GROUP_SIDE_PANEL, 0);
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

static void draw_military_panel_foreground(int x_offset)
{
    const formation *m = formation_get(data.active_legion.formation_id);
    if (has_legion_changed(&data.active_legion, m)) {
        draw_military_panel_background(x_offset);
    }
    int y_offset = Y_OFFSET_PANEL_START;
    int num_legions = formation_get_num_legions();
    if (num_legions > 1) {
        arrow_buttons_draw(x_offset, y_offset, buttons_cycle_legion, 2);
    }
    if (m->num_figures) {
        draw_layout_buttons(x_offset, y_offset + Y_OFFSET_LAYOUT_BUTTONS, 0, m);
        for (int i = 0; i < 3; i++) {
            button_border_draw(x_offset + buttons_bottom[i].x, y_offset + Y_OFFSET_BOTTOM_BUTTONS,
                30, 30, data.bottom_buttons_focus_id == i + 1);
        }
    }
}

static void draw_foreground(int x_offset)
{
    widget_minimap_draw(x_offset + 8, 59, MINIMAP_WIDTH, MINIMAP_HEIGHT, 0);
    image_buttons_draw(x_offset, 24, buttons_title_close, 2);
    lang_text_draw_centered(61, 5, x_offset, 32, 117, FONT_NORMAL_GREEN);
    draw_military_panel_foreground(x_offset);
    sidebar_extra_draw_foreground();
}

void widget_sidebar_military_draw_foreground(void)
{
    draw_foreground(sidebar_common_get_x_offset_expanded());
}

static void draw_sliding(int x_offset)
{
    draw_background(x_offset);
    draw_foreground(x_offset);
}

int widget_sidebar_military_handle_input(const mouse *m)
{
    int x_offset = sidebar_common_get_x_offset_expanded();
    if (image_buttons_handle_mouse(m, x_offset, 24, buttons_title_close, 2, &data.top_buttons_focus_id)) {
        return 1;
    }
    int num_legions = formation_get_num_legions();
    if (num_legions > 1 &&
        arrow_buttons_handle_mouse(m, x_offset, Y_OFFSET_PANEL_START, buttons_cycle_legion, 2, 0)) {
        return 1;
    }
    const formation *selected_legion = formation_get(data.active_legion.formation_id);
    if (data.active_legion.soldiers > 0) {
        generic_button *layout_buttons = buttons_formation_layout[available_layouts_for_legion(selected_legion) - 3];
        if (generic_buttons_handle_mouse(m, x_offset, Y_OFFSET_PANEL_START + Y_OFFSET_LAYOUT_BUTTONS,
            layout_buttons, 5, &data.inner_buttons_focus_id)) {
            return 1;
        }
        if (generic_buttons_handle_mouse(m, x_offset, Y_OFFSET_PANEL_START + Y_OFFSET_BOTTOM_BUTTONS,
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
    widget_minimap_invalidate();
}

static void slide_in_finished(void)
{
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
    }
    widget_minimap_invalidate();
    window_city_return();
}

static void slide_out_finished(void)
{
    data.active_legion.formation_id = 0;
    widget_minimap_invalidate();
    window_city_show();
}

int widget_sidebar_military_enter(int formation_id)
{
    clear_focus_buttons();
    int had_selected_legions = data.active_legion.formation_id;
    set_formation_id(formation_id);
    if (had_selected_legions) {
        return 0;
    }
    data.city_view_was_collapsed = city_view_is_sidebar_collapsed();
    if (data.city_view_was_collapsed) {
        city_view_start_sidebar_toggle();
        sidebar_slide(SLIDE_DIRECTION_IN, widget_sidebar_city_draw_background, draw_sliding, slide_in_finished);
    } else {
        slide_in_finished();
    }
    return 1;
}

int widget_sidebar_military_exit(void)
{
    clear_focus_buttons();
    if (!window_is(WINDOW_CITY_MILITARY)) {
        widget_minimap_invalidate();
        return 0;
    }
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
        sidebar_slide(SLIDE_DIRECTION_OUT, widget_sidebar_city_draw_background, draw_sliding, slide_out_finished);
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
