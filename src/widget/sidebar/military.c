#include "military.h"

#include "building/count.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/log.h"
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

#define Y_OFFSET_PANEL_START 175
#define MILITARY_PANEL_BLOCKS 19

typedef struct {
    int formation_id;
    int soldiers;
    int health;
    int morale;
    int layout;
    int trained;
} legion_info;

static struct {
    legion_info selected_legions[MAX_LEGIONS];
    legion_info *active_legion;
    int total_selected_legions;
    int top_buttons_focus_id;
    int inner_buttons_focus_id;
    int bottom_buttons_focus_id;
    int double_click;
    int city_view_was_collapsed;
} data;

static void button_military_menu(int param1, int param2);
static void button_close_military_sidebar(int param1, int param2);
static void button_cycle_legion(int cycle_forward, int param2);
static void button_select_formation_layout(int index, int param2);
static void button_go_to_legion(int legion_id, int param2);
static void button_return_to_fort(int param1, int param2);
static void button_select_legion(int legion, int param2);

static image_button buttons_title_close[] = {
    {127, 5, 31, 20, IB_NORMAL, 90, 0, button_close_military_sidebar, button_none, 0, 0, 1},
    {4, 3, 117, 31, IB_NORMAL, 93, 0, button_military_menu, button_none, 0, 0, 1}
};

static arrow_button buttons_cycle_legion[] = {
    {1, 0, 19, 24, button_cycle_legion, 0, 0},
    {125, 0, 21, 24, button_cycle_legion, 1, 0},
};

static generic_button buttons_formation_layout[] = {
    {0, 0, 29, 29, button_select_formation_layout, button_none, 0, 0},
    {31, 0, 29, 29, button_select_formation_layout, button_none, 1, 0},
    {62, 0, 29, 29, button_select_formation_layout, button_none, 2, 0},
    {93, 0, 29, 29, button_select_formation_layout, button_none, 3, 0},
    {124, 0, 29, 29, button_select_formation_layout, button_none, 4, 0}
};

static generic_button buttons_single_legion_bottom[] = {
    {0, 0, 73, 48, button_go_to_legion, button_none, 0, 0},
    {77, 0, 73, 48, button_return_to_fort, button_none, 1, 0},
};

static generic_button buttons_select_legion[] = {
    {0, 0, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 0, 0},
    {0, 50, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 1, 0},
    {0, 100, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 2, 0},
    {0, 150, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 3, 0},
    {0, 200, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 4, 0},
    {0, 250, SIDEBAR_EXPANDED_WIDTH - 12, 50, button_select_legion, button_none, 5, 0},
};

static const int IMAGE_OFFSETS_TO_FORMATION[7] = {
    FORMATION_COLUMN,
    FORMATION_TORTOISE,
    FORMATION_DOUBLE_LINE_1,
    FORMATION_DOUBLE_LINE_2,
    FORMATION_MOP_UP,
    FORMATION_SINGLE_LINE_1,
    FORMATION_SINGLE_LINE_2
};

static const int LAYOUT_IMAGE_OFFSETS_LEGIONARY[2][FORMATIONS_PER_LEGION] = {
    {0, 0, 2, 3, 4}, {0, 0, 3, 2, 4},
};

static const int LAYOUT_IMAGE_OFFSETS_OTHER[2][FORMATIONS_PER_LEGION] = {
    {5, 6, 2, 3, 4}, {6, 5, 3, 2, 4},
};

static const int LAYOUT_BUTTON_INDEXES_LEGIONARY[2][FORMATIONS_PER_LEGION] = {
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

static const int LAYOUT_BUTTON_INDEXES_OTHER[2][FORMATIONS_PER_LEGION] = {
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

static void column_formation_button(int x, int y, int selected)
{
    color_t bg_color = selected ? COLOR_FORMATION_BUTTON_2 : COLOR_FORMATION_BUTTON_1;
    color_t fg_color = selected ? COLOR_FORMATION_BUTTON_1 : COLOR_FORMATION_BUTTON_2;

    graphics_fill_rect(x, y, 23, 23, bg_color);

    graphics_fill_rect(x + 4, y + 4, 2, 2, fg_color);
    graphics_fill_rect(x + 10, y + 4, 2, 2, fg_color);
    graphics_fill_rect(x + 16, y + 4, 2, 2, fg_color);

    graphics_fill_rect(x + 4, y + 10, 2, 2, fg_color);
    graphics_fill_rect(x + 10, y + 10, 2, 2, fg_color);
    graphics_fill_rect(x + 16, y + 10, 2, 2, fg_color);

    graphics_fill_rect(x + 4, y + 16, 2, 2, fg_color);
    graphics_fill_rect(x + 10, y + 16, 2, 2, fg_color);
    graphics_fill_rect(x + 16, y + 16, 2, 2, fg_color);
}

static void single_line_formation_button(int x, int y, int invert, int selected)
{
    color_t bg_color = selected ? COLOR_FORMATION_BUTTON_2 : COLOR_FORMATION_BUTTON_1;
    color_t fg_color = selected ? COLOR_FORMATION_BUTTON_1 : COLOR_FORMATION_BUTTON_2;

    graphics_fill_rect(x, y, 23, 23, bg_color);

    if (invert) {
        graphics_fill_rect(x + 18, y + 3, 2, 2, fg_color);
        graphics_fill_rect(x + 13, y + 8, 2, 2, fg_color);
        graphics_fill_rect(x + 8, y + 13, 2, 2, fg_color);
        graphics_fill_rect(x + 3, y + 18, 2, 2, fg_color);
    } else {
        graphics_fill_rect(x + 3, y + 3, 2, 2, fg_color);
        graphics_fill_rect(x + 8, y + 8, 2, 2, fg_color);
        graphics_fill_rect(x + 13, y + 13, 2, 2, fg_color);
        graphics_fill_rect(x + 18, y + 18, 2, 2, fg_color);
    }
}

static void double_line_formation_button(int x, int y, int invert, int selected)
{
    color_t bg_color = selected ? COLOR_FORMATION_BUTTON_2 : COLOR_FORMATION_BUTTON_1;
    color_t fg_color = selected ? COLOR_FORMATION_BUTTON_1 : COLOR_FORMATION_BUTTON_2;

    graphics_fill_rect(x, y, 23, 23, bg_color);

    if (invert) {
        graphics_fill_rect(x + 19, y + 3, 2, 2, fg_color);
        graphics_fill_rect(x + 15, y + 3, 2, 2, fg_color);

        graphics_fill_rect(x + 15, y + 8, 2, 2, fg_color);
        graphics_fill_rect(x + 11, y + 8, 2, 2, fg_color);

        graphics_fill_rect(x + 11, y + 13, 2, 2, fg_color);
        graphics_fill_rect(x + 7, y + 13, 2, 2, fg_color);

        graphics_fill_rect(x + 7, y + 18, 2, 2, fg_color);
        graphics_fill_rect(x + 3, y + 18, 2, 2, fg_color);
    } else {
        graphics_fill_rect(x + 3, y + 3, 2, 2, fg_color);
        graphics_fill_rect(x + 7, y + 3, 2, 2, fg_color);

        graphics_fill_rect(x + 7, y + 8, 2, 2, fg_color);
        graphics_fill_rect(x + 11, y + 8, 2, 2, fg_color);

        graphics_fill_rect(x + 11, y + 13, 2, 2, fg_color);
        graphics_fill_rect(x + 15, y + 13, 2, 2, fg_color);

        graphics_fill_rect(x + 15, y + 18, 2, 2, fg_color);
        graphics_fill_rect(x + 19, y + 18, 2, 2, fg_color);
    }
}

static void mop_up_formation_button(int x, int y, int selected)
{
    color_t bg_color = selected ? COLOR_FORMATION_BUTTON_2 : COLOR_FORMATION_BUTTON_1;
    color_t fg_color = selected ? COLOR_FORMATION_BUTTON_1 : COLOR_FORMATION_BUTTON_2;

    graphics_fill_rect(x, y, 23, 23, bg_color);

    graphics_fill_rect(x + 4, y + 4, 2, 2, fg_color);
    graphics_fill_rect(x + 10, y + 7, 2, 2, fg_color);
    graphics_fill_rect(x + 16, y + 4, 2, 2, fg_color);

    graphics_draw_vertical_line(x + 4, y + 10, y + 11, fg_color);
    graphics_draw_vertical_line(x + 3, y + 12, y + 15, fg_color);
    graphics_draw_horizontal_line(x + 2, x + 4, y + 14, fg_color);

    graphics_draw_vertical_line(x + 10, y + 13, y + 19, fg_color);
    graphics_draw_horizontal_line(x + 9, x + 11, y + 18, fg_color);

    graphics_draw_vertical_line(x + 17, y + 10, y + 11, fg_color);
    graphics_draw_vertical_line(x + 18, y + 12, y + 15, fg_color);
    graphics_draw_horizontal_line(x + 17, x + 19, y + 14, fg_color);
}

static void draw_layout_button(int button_id, int x, int y, int selected)
{
    switch (button_id) {
        case 0:
            column_formation_button(x, y, selected);
            return;
        case 2:
            double_line_formation_button(x, y, 0, selected);
            return;
        case 3:
            double_line_formation_button(x, y, 1, selected);
            return;
        case 4:
            mop_up_formation_button(x, y, selected);
            return;
        case 5:
            single_line_formation_button(x, y, 0, selected);
            return;
        case 6:
            single_line_formation_button(x, y, 1, selected);
    }
}

static void draw_all_layout_buttons(int x, int y, int draw_buttons, const formation *m)
{
    int index = 0;
    if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
        index = 1;
    }
    const int *offsets = LAYOUT_IMAGE_OFFSETS_OTHER[index];
    int formation_types = FORMATIONS_PER_LEGION;

    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        offsets = LAYOUT_IMAGE_OFFSETS_LEGIONARY[index];
        if (m->has_military_training) {
            formation_types = FORMATIONS_PER_LEGION - 1;
        } else {
            formation_types = FORMATIONS_PER_LEGION - 2;
        }
    }
    int start_formation = FORMATIONS_PER_LEGION - formation_types;
    for (int i = start_formation; i < FORMATIONS_PER_LEGION; i++) {
        button_border_draw(x, y, 29, 29, i == data.inner_buttons_focus_id - 1 + start_formation);
        if (draw_buttons) {
            draw_layout_button(offsets[i], x + 3, y + 3, m->layout == IMAGE_OFFSETS_TO_FORMATION[offsets[i]]);
        }
        x += 31;
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

static void draw_military_info_single(int x_offset, int y_offset)
{
    legion_info *l = data.active_legion;
    const formation *m = formation_get(l->formation_id);
    int formation_image = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;

    // Legion name (and optionally legion cycle buttons)
    if (formation_get_num_legions() == 1) {
        image_draw(formation_image, x_offset, y_offset + 10);
        lang_text_draw_centered(138, m->legion_id, x_offset + image_get(formation_image)->width, y_offset + 14, SIDEBAR_EXPANDED_WIDTH - 6 - image_get(formation_image)->width, FONT_NORMAL_WHITE);
    } else {
        arrow_buttons_draw(x_offset, y_offset + 6, buttons_cycle_legion, 2);
        image_draw(formation_image, x_offset + (SIDEBAR_EXPANDED_WIDTH - 12 - image_get(formation_image)->width) / 2, y_offset + 8);
        lang_text_draw_centered(138, m->legion_id, x_offset, y_offset + 34, SIDEBAR_EXPANDED_WIDTH - 12, FONT_NORMAL_WHITE);
        y_offset += 20;
    }

    // Number of soldiers
    int width = text_draw_number(m->num_figures, '@', " ", x_offset, y_offset + 40, FONT_NORMAL_WHITE);
    lang_text_draw(138, 46 - m->figure_type, x_offset + width, y_offset + 40, FONT_NORMAL_WHITE);

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
        lang_text_draw_multiline(group_id, text_id, x_offset, y_offset + 66, SIDEBAR_EXPANDED_WIDTH - 12, FONT_NORMAL_WHITE);
        l->health = 0;
        l->layout = 0;
        l->morale = 0;
        l->soldiers = 0;
        l->trained = 0;
        return;
    }

    // Morale
    lang_text_draw(138, 36, x_offset, y_offset + 66, FONT_NORMAL_WHITE);
    lang_text_draw(138, 37 + m->morale / 5, x_offset + 4, y_offset + 86, m->morale < 13 ? FONT_NORMAL_RED : FONT_NORMAL_GREEN);

    // Health
    int health = calc_percentage(m->total_damage, m->max_total_damage);
    lang_text_draw(138, 24, x_offset, y_offset + 108, FONT_NORMAL_WHITE);
    lang_text_draw(138, get_health_text_id(health), x_offset + 4, y_offset + 128, health < 55 ? FONT_NORMAL_GREEN : FONT_NORMAL_RED);

    // Military training
    lang_text_draw(138, 25, x_offset, y_offset + 150, FONT_NORMAL_WHITE);
    lang_text_draw(18, m->has_military_training, x_offset + 4, y_offset + 170, m->has_military_training ? FONT_NORMAL_GREEN : FONT_NORMAL_RED);

    // Formation layout
    draw_all_layout_buttons(x_offset, y_offset + 192, 1, m);

    // Go to legion button
    button_border_draw(x_offset, y_offset + 228, 73, 48, data.bottom_buttons_focus_id == 1);
    lang_text_draw_centered(51, 1, x_offset + 2, y_offset + 238, 71, FONT_NORMAL_WHITE);
    lang_text_draw_centered(51, 2, x_offset + 2, y_offset + 254, 71, FONT_NORMAL_WHITE);

    // Return to fort button
    button_border_draw(x_offset + 77, y_offset + 228, 73, 48, data.bottom_buttons_focus_id == 2);
    lang_text_draw_centered(51, 3, x_offset + 79, y_offset + 238, 71, FONT_NORMAL_WHITE);
    lang_text_draw_centered(51, 4, x_offset + 79, y_offset + 254, 71, FONT_NORMAL_WHITE);

    l->health = health;
    l->layout = m->layout;
    l->morale = m->morale;
    l->soldiers = m->num_figures;
    l->trained = m->has_military_training;
}

static void draw_military_info_multiple(int x_offset, int y_offset)
{
    y_offset += 3;
    for (int i = 0; i < data.total_selected_legions; ++i) {
        legion_info *l = &data.selected_legions[i];
        const formation *m = formation_get(l->formation_id);

        // Legion icon
        int image_id = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;
        int icon_height = image_get(image_id)->height;
        image_draw(image_id, x_offset + 2, y_offset + 2);

        // Legion flag
        image_id = image_group(GROUP_FIGURE_FORT_FLAGS) + 8;
        if (m->figure_type == FIGURE_FORT_JAVELIN) {
            image_id += 9;
        } else if (m->figure_type == FIGURE_FORT_MOUNTED) {
            image_id += 18;
        }
        image_draw(image_id, x_offset + 2, y_offset + 2 + icon_height);

        // Number of soldiers
        lang_text_draw_amount(8, 46, m->num_figures, x_offset + 35, y_offset + 10, FONT_NORMAL_WHITE);

        // Morale
        lang_text_draw(138, 37 + m->morale / 5, x_offset + 40, y_offset + 30, m->morale < 13 ? FONT_NORMAL_RED : FONT_NORMAL_GREEN);

        button_border_draw(x_offset, y_offset, SIDEBAR_EXPANDED_WIDTH - 12, 50, l == data.active_legion || data.inner_buttons_focus_id == i + 1);

        y_offset += 50;
    }
}

static void draw_military_panel_background(int x_offset)
{
    graphics_draw_vertical_line(x_offset, Y_OFFSET_PANEL_START, Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16, COLOR_WHITE);
    graphics_draw_vertical_line(x_offset + SIDEBAR_EXPANDED_WIDTH - 1, Y_OFFSET_PANEL_START, Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16, COLOR_SIDEBAR);
    inner_panel_draw(x_offset + 1, Y_OFFSET_PANEL_START, SIDEBAR_EXPANDED_WIDTH / 16, MILITARY_PANEL_BLOCKS);

    if (data.total_selected_legions == 1) {
        draw_military_info_single(x_offset + 6, Y_OFFSET_PANEL_START);
    } else {
        draw_military_info_multiple(x_offset + 6, Y_OFFSET_PANEL_START);
    }
}

static void set_minimap_selected_formations(void)
{
    static int formations[MAX_LEGIONS];
    for (int i = 0; i < data.total_selected_legions; ++i) {
        formations[i] = data.selected_legions[i].formation_id;
    }
    widget_minimap_set_selected_formations(formations, data.total_selected_legions);
    widget_minimap_invalidate();
}

static void draw_background(int x_offset)
{
    image_draw(image_group(GROUP_SIDE_PANEL) + 1, x_offset, 24);
    image_buttons_draw(x_offset, 24, buttons_title_close, 2);
    lang_text_draw_centered(61, 5, x_offset, 32, 117, FONT_NORMAL_GREEN);
    widget_minimap_draw(x_offset + 8, 59, 73, 111, 1);
    draw_military_panel_background(x_offset);
    int panel_height = Y_OFFSET_PANEL_START + MILITARY_PANEL_BLOCKS * 16;
    int extra_height = sidebar_extra_draw_background(x_offset, panel_height, SIDEBAR_EXPANDED_WIDTH, sidebar_common_get_height() - panel_height + TOP_MENU_HEIGHT, 0, SIDEBAR_EXTRA_DISPLAY_GAME_SPEED);
    sidebar_extra_draw_foreground();

    sidebar_common_draw_relief(x_offset, panel_height + extra_height, GROUP_SIDE_PANEL, 0);
}

void widget_sidebar_military_draw_background(void)
{
    draw_background(sidebar_common_get_x_offset_expanded());
}

static int has_legion_changed(const legion_info *l, const formation *m)
{
    return l->health != calc_percentage(m->total_damage, m->max_total_damage) ||
        l->layout != m->layout ||
        l->morale != m->morale ||
        l->soldiers != m->num_figures ||
        l->trained != m->has_military_training;
}

static void draw_military_panel_foreground(void)
{
    int x_offset = sidebar_common_get_x_offset_expanded();
    if (data.total_selected_legions == 1) {
        int num_legions = formation_get_num_legions();
        if (num_legions > 1) {
            arrow_buttons_draw(x_offset + 6, 181, buttons_cycle_legion, 2);
        }
        const formation *m = formation_get(data.active_legion->formation_id);
        if (has_legion_changed(data.active_legion, m)) {
            draw_military_panel_background(x_offset);
            return;
        }
        if (!m->num_figures) {
            return;
        }
        int extra_y_offset = num_legions > 1 ? 20 : 0;
        draw_all_layout_buttons(x_offset + 6, Y_OFFSET_PANEL_START + extra_y_offset + 192, 0, m);
        button_border_draw(x_offset + 6, Y_OFFSET_PANEL_START + extra_y_offset + 228, 73, 48, data.bottom_buttons_focus_id == 1);
        button_border_draw(x_offset + 83, Y_OFFSET_PANEL_START + extra_y_offset + 228, 73, 48, data.bottom_buttons_focus_id == 2);
    } else {
        for (int i = 0; i < data.total_selected_legions; ++i) {
            legion_info *l = &data.selected_legions[i];
            const formation *m = formation_get(l->formation_id);
            l->health = calc_percentage(m->total_damage, m->max_total_damage);
            l->layout = m->layout;
            l->trained = m->has_military_training;
            if (has_legion_changed(l, m)) {
                draw_military_panel_background(x_offset);
                return;
            }
        }
    }
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
    if (!config_get(CONFIG_UI_SHOW_MILITARY_SIDEBAR)) {
        return 0;
    }
    data.double_click = m->left.double_click;
    int x_offset = sidebar_common_get_x_offset_expanded();
    if (image_buttons_handle_mouse(m, x_offset, 24, buttons_title_close, 2, &data.top_buttons_focus_id)) {
        return 1;
    }
    if (data.total_selected_legions == 1) {
        int num_legions = formation_get_num_legions();
        int extra_y_offset = num_legions > 1 ? 20 : 0;
        if (num_legions > 1 &&
            arrow_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + 6, buttons_cycle_legion, 2, 0)) {
            return 1;
        }
        if (generic_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + extra_y_offset + 192, buttons_formation_layout, 5, &data.inner_buttons_focus_id)) {
            return 1;
        }
        if (generic_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + extra_y_offset + 228, buttons_single_legion_bottom, 2, &data.bottom_buttons_focus_id)) {
            return 1;
        }
    } else {
        if (generic_buttons_handle_mouse(m, x_offset + 6, Y_OFFSET_PANEL_START + 3, buttons_select_legion, data.total_selected_legions, &data.inner_buttons_focus_id)) {
            return 1;
        }
    }
    return 0;
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
            log_info("Unknown formation", 0, layout);
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
        if (data.total_selected_legions == 1) {
            int index = data.inner_buttons_focus_id - 1;
            int layout;
            const formation *m = formation_get(data.active_legion->formation_id);
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                int index_increase = m->has_military_training ? 1 : 2;
                if (index > 4 - index_increase) {
                    return 0;
                }
                index += index_increase;
                layout = LAYOUT_BUTTON_INDEXES_LEGIONARY[0][index];
            } else {
                layout = LAYOUT_BUTTON_INDEXES_OTHER[0][index];
            }
            c->text_group = 138;
            return get_layout_text_id(layout);
        } else {
            c->text_group = 137;
            return formation_get(data.selected_legions[data.inner_buttons_focus_id - 1].formation_id)->legion_id + 2;
        }
    }
    return 0;
}

void widget_sidebar_military_set_single_formation_id(int formation_id)
{
    data.active_legion = &data.selected_legions[0];
    data.active_legion->formation_id = formation_id;
    data.total_selected_legions = 1;
}

void widget_sidebar_military_select_all(void)
{
    int active_formation = data.active_legion->formation_id;
    int num_legions = formation_get_num_legions();
    int current_legion = 1;
    for (int i = 1; i < MAX_FORMATIONS; ++i) {
        const formation *m = formation_get(i);
        if (m->in_use != 1 || m->is_herd || !m->is_legion) {
            continue;
        }
        legion_info *l = &data.selected_legions[current_legion - 1];
        l->formation_id = i;
        l->health = 0;
        l->layout = 0;
        l->morale = 0;
        l->soldiers = 0;
        l->trained = 0;
        if (active_formation == i) {
            data.active_legion = l;
        }
        current_legion++;
        if (current_legion > num_legions) {
            break;
        }
    }
    data.total_selected_legions = num_legions;
    set_minimap_selected_formations();
}

static void slide_in_finished(void)
{
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
    }
    set_minimap_selected_formations();
    window_city_return();
    window_draw(1);
}

static void slide_out_finished(void)
{
    data.total_selected_legions = 0;
    set_minimap_selected_formations();
    window_city_show();
    window_draw(1);
}

int widget_sidebar_military_enter(int formation_id)
{
    int had_selected_legions = data.total_selected_legions;
    widget_sidebar_military_set_single_formation_id(formation_id);
    if (had_selected_legions) {
        set_minimap_selected_formations();
        return 0;
    }
    data.city_view_was_collapsed = city_view_is_sidebar_collapsed();
    if (data.city_view_was_collapsed) {
        city_view_start_sidebar_toggle();
    }
    sidebar_slide(SLIDE_DIRECTION_IN, widget_sidebar_city_draw_background, draw_background, slide_in_finished);
    return 1;
}

int widget_sidebar_military_exit(void)
{
    if (!window_is(WINDOW_CITY_MILITARY)) {
        data.total_selected_legions = 0;
        set_minimap_selected_formations();
        return 0;
    }
    if (data.city_view_was_collapsed) {
        city_view_toggle_sidebar();
    }
    sidebar_slide(SLIDE_DIRECTION_OUT, widget_sidebar_city_draw_background, draw_background, slide_out_finished);
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
    legion_info *l = data.active_legion;
    int step = cycle_forward ? 1 : -1;
    const formation *m;
    for (int i = l->formation_id + step; i != l->formation_id; i += step) {
        if (i == 0) {
            i = MAX_FORMATIONS;
        } else if (i > MAX_FORMATIONS) {
            i = 1;
        }
        m = formation_get(i);
        if (m->in_use == 1 && !m->is_herd && m->is_legion) {
            l->formation_id = i;
            break;
        }
    }
    window_city_military_set_formation_id(l->formation_id);
    widget_sidebar_military_set_single_formation_id(l->formation_id);
    set_minimap_selected_formations();
}

static void button_select_formation_layout(int index, int param2)
{
    formation *m = formation_get(data.active_legion->formation_id);
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
        layout_indexes = LAYOUT_BUTTON_INDEXES_OTHER[swap_lines];
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

static void button_go_to_legion(int formation_id, int param2)
{
    if (formation_id == 0) {
        formation_id = data.active_legion->formation_id;
    }
    const formation *m = formation_get(formation_id);
    city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
}

static void button_return_to_fort(int param1, int param2)
{
    formation *m = formation_get(data.active_legion->formation_id);
    if (!m->in_distant_battle) {
        formation_legion_return_home(m);
    }
}

static void button_select_legion(int legion, int param2)
{
    if (data.double_click) {
        button_go_to_legion(data.selected_legions[legion].formation_id, 0);
        window_city_military_show(data.selected_legions[legion].formation_id);
    } else {
        data.active_legion = &data.selected_legions[legion];
        window_city_military_set_formation_id(data.active_legion->formation_id);
    }
}
