#include "military.h"

#include "building/barracks.h"
#include "building/building.h"
#include "building/count.h"
#include "city/data_private.h"
#include "city/buildings.h"
#include "city/military.h"
#include "city/view.h"
#include "core/calc.h"
#include "core/log.h"
#include "core/string.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/city.h"
#include "window/building/culture.h"

static void button_return_to_fort(int param1, int param2);
static void button_layout(int index, int param2);
static void button_priority(int index, int param2);


static generic_button layout_buttons[] = {
    {19, 179, 84, 84, button_layout, button_none, 0, 0},
    {104, 179, 84, 84, button_layout, button_none, 1, 0},
    {189, 179, 84, 84, button_layout, button_none, 2, 0},
    {274, 179, 84, 84, button_layout, button_none, 3, 0},
    {359, 179, 84, 84, button_layout, button_none, 4, 0}
};

static generic_button priority_buttons[] = {
    {96, 0, 24, 24, button_priority, button_none, 0, 0},
    {96, 24, 24, 24, button_priority, button_none, 1, 0},
};


static generic_button return_button[] = {
    {0, 0, 288, 32, button_return_to_fort, button_none, 0, 0},
};

static struct {
    int focus_button_id;
    int focus_priority_button_id;
    int return_button_id;
    int building_id;
    building_info_context *context_for_callback;
} data;

static void draw_priority_buttons(int x, int y, int buttons)
{
    uint8_t permission_selection_text[] = { 'x', 0 };
    for (int i = 0; i < buttons; i++) {
        int x_adj = x + priority_buttons[i].x;
        int y_adj = y + priority_buttons[i].y;
        building *barracks = building_get(data.building_id);
        int priority = building_barracks_get_priority(barracks);
        button_border_draw(x_adj, y_adj, 20, 20, data.focus_priority_button_id == i + 1 ? 1 : 0);
        if (priority == i) {
            text_draw_centered(permission_selection_text, x_adj + 1, y_adj + 4, 20, FONT_NORMAL_BLACK, 0);
        }
    }
}

void window_building_draw_wall(building_info_context *c)
{
    c->help_id = 85;
    window_building_play_sound(c, "wavs/wall.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(139, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 158, 139, 1);
}

void window_building_draw_gatehouse(building_info_context *c)
{
    c->help_id = 85;
    window_building_play_sound(c, "wavs/gatehouse.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(90, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 158, 90, 1);
}

void window_building_draw_tower(building_info_context *c)
{
    c->help_id = 85;
    window_building_play_sound(c, "wavs/tower.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(91, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 91, 2);
    } else if (b->figure_id) {
        window_building_draw_description(c, 91, 3);
    } else {
        window_building_draw_description(c, 91, 4);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_barracks(building_info_context *c)
{
    c->help_id = 37;
    data.building_id = c->building_id;
    window_building_play_sound(c, "wavs/barracks.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(136, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    image_draw(resource_get_data(RESOURCE_WEAPONS)->image.icon, c->x_offset + 64, c->y_offset + 38,
        COLOR_MASK_NONE, SCALE_NONE);

    building *b = building_get(c->building_id);
    if (b->loads_stored < 1) {
        lang_text_draw_amount(8, 10, 0, c->x_offset + 92, c->y_offset + 44, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount(8, 10, b->loads_stored, c->x_offset + 92, c->y_offset + 44, FONT_NORMAL_BLACK);
    }

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 70, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 70, 136, 3);
    } else if (!c->barracks_soldiers_requested) {
        window_building_draw_description_at(c, 70, 136, 4);
    } else {
        int offset = 0;
        if (b->loads_stored > 0) {
            offset = 4;
        }
        if (city_data.mess_hall.food_stress_cumulative > 50) {
            text_draw_multiline(translation_for(TR_BUILDING_BARRACKS_FOOD_WARNING_2),
                c->x_offset + 26, c->y_offset + 86, 16 * c->width_blocks - 30, FONT_NORMAL_BLACK, 0);
        } else if (city_data.mess_hall.food_stress_cumulative > 20) {
            text_draw_multiline(translation_for(TR_BUILDING_BARRACKS_FOOD_WARNING),
                c->x_offset + 26, c->y_offset + 86, 16 * c->width_blocks - 30, FONT_NORMAL_BLACK, 0);
        } else if (c->worker_percentage >= 100) {
            window_building_draw_description_at(c, 70, 136, 5 + offset);
        } else if (c->worker_percentage >= 66) {
            window_building_draw_description_at(c, 70, 136, 6 + offset);
        } else if (c->worker_percentage >= 33) {
            window_building_draw_description_at(c, 70, 136, 7 + offset);
        } else {
            window_building_draw_description_at(c, 70, 136, 8 + offset);
        }
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    lang_text_draw(50, 21, c->x_offset + 46, c->y_offset + 204, FONT_NORMAL_BLACK); // "Priority"
    lang_text_draw(91, 0, c->x_offset + 46, c->y_offset + 224, FONT_NORMAL_BLACK); // "Tower"
    lang_text_draw(89, 0, c->x_offset + 46, c->y_offset + 244, FONT_NORMAL_BLACK); // "Fort"
}

void window_building_draw_barracks_foreground(building_info_context *c)
{
    draw_priority_buttons(c->x_offset + 46, c->y_offset + 224, 2);

}

void window_building_draw_priority_buttons(int x, int y)
{
    draw_priority_buttons(x, y, 2);
}

int window_building_handle_mouse_barracks(const mouse *m, building_info_context *c)
{
    if (generic_buttons_handle_mouse(m, c->x_offset + 46, c->y_offset + 224,
        priority_buttons, 2, &data.focus_priority_button_id)) {
        window_invalidate();
        return 1;
    }

    return 0;
}

int window_building_handle_mouse_grand_temple_mars(const mouse *m, building_info_context *c)
{
    if (generic_buttons_handle_mouse(m, c->x_offset + 285, c->y_offset + 55,
        priority_buttons, 2, &data.focus_priority_button_id)) {
        window_invalidate();
        return 1;
    }

    window_building_handle_mouse_grand_temple(m, c);
    return 0;
}

void window_building_draw_military_academy(building_info_context *c)
{
    c->help_id = 88;
    window_building_play_sound(c, "wavs/mil_acad.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(135, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 135, 2);
    } else if (c->worker_percentage >= 100) {
        window_building_draw_description(c, 135, 1);
    } else {
        window_building_draw_description(c, 135, 3);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_fort(building_info_context *c)
{
    c->help_id = 87;
    window_building_play_sound(c, "wavs/fort.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(89, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    int text_id = formation_get(c->formation_id)->cursed_by_mars ? 1 : 2;
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 208, 89, text_id);

    building *b = building_get(c->building_id);
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    if (building_get_levy(b)) {
        window_building_draw_levy(building_get_levy(b), c->x_offset, c->y_offset + 150);
    }
}

void window_building_draw_legion_info(building_info_context *c)
{
    int text_id;
    int food_stress = city_mess_hall_food_stress();
    int hunger_text;
    const formation *m = formation_get(c->formation_id);
    c->help_id = 87;
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(138, m->legion_id, c->x_offset, c->y_offset + 10,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    // standard icon at the top
    int icon_image_id = image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id;
    const image *icon_image = image_get(icon_image_id);
    int icon_height = icon_image->height;
    image_draw(icon_image_id, c->x_offset + 16 + (40 - icon_image->width - icon_image->x_offset) / 2, c->y_offset + 16,
        COLOR_MASK_NONE, SCALE_NONE);
    // standard flag
    int flag_image_id = image_group(GROUP_FIGURE_FORT_FLAGS);
    if (m->figure_type == FIGURE_FORT_JAVELIN) {
        flag_image_id += 9;
    } else if (m->figure_type == FIGURE_FORT_MOUNTED) {
        flag_image_id += 18;
    }
    if (m->is_halted) {
        flag_image_id += 8;
    }
    const image *flag_image = image_get(flag_image_id);
    int flag_height = flag_image->height;
    image_draw(flag_image_id, c->x_offset + 16 + (40 - flag_image->width - flag_image->x_offset) / 2,
        c->y_offset + 16 + icon_height, COLOR_MASK_NONE, SCALE_NONE);
    // standard pole and morale ball
    int morale_offset = m->morale / 5;
    if (morale_offset > 20) {
        morale_offset = 20;
    }
    int pole_image_id = image_group(GROUP_FIGURE_FORT_STANDARD_POLE) + 20 - morale_offset;
    const image *pole_image = image_get(pole_image_id);
    image_draw(pole_image_id, c->x_offset + 16 + (40 - pole_image->width - pole_image->x_offset * 2) / 2,
        c->y_offset + 16 + icon_height + flag_height, COLOR_MASK_NONE, SCALE_NONE);

    // number of soldiers
    lang_text_draw(138, 23, c->x_offset + 100, c->y_offset + 60, FONT_NORMAL_BLACK);
    text_draw_number(m->num_figures, '@', " ", c->x_offset + 294, c->y_offset + 60, FONT_NORMAL_BLACK, 0);
    // health
    lang_text_draw(138, 24, c->x_offset + 100, c->y_offset + 80, FONT_NORMAL_BLACK);
    if (m->mess_hall_max_morale_modifier < -20) {
        text_draw(translation_for(TR_BUILDING_LEGION_STARVING),
            c->x_offset + 300, c->y_offset + 80, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    } else {
        int health = calc_percentage(m->total_damage, m->max_total_damage);
        if (health <= 0) {
            text_id = 26;
        } else if (health <= 20) {
            text_id = 27;
        } else if (health <= 40) {
            text_id = 28;
        } else if (health <= 55) {
            text_id = 29;
        } else if (health <= 70) {
            text_id = 30;
        } else if (health <= 90) {
            text_id = 31;
        } else {
            text_id = 32;
        }
        lang_text_draw(138, text_id, c->x_offset + 300, c->y_offset + 80, FONT_NORMAL_BLACK);
    }
    // military training
    lang_text_draw(138, 25, c->x_offset + 100, c->y_offset + 100, FONT_NORMAL_BLACK);
    lang_text_draw(18, m->has_military_training, c->x_offset + 300, c->y_offset + 100, FONT_NORMAL_BLACK);
    // morale
    if (m->cursed_by_mars) {
        lang_text_draw(138, 59, c->x_offset + 100, c->y_offset + 120, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw(138, 36, c->x_offset + 100, c->y_offset + 120, FONT_NORMAL_BLACK);
        lang_text_draw(138, 37 + morale_offset, c->x_offset + 300, c->y_offset + 120, FONT_NORMAL_BLACK);
    }
    // food
    text_draw(translation_for(TR_BUILDING_LEGION_FOOD_STATUS),
        c->x_offset + 100, c->y_offset + 140, FONT_NORMAL_BLACK, 0);
    if (food_stress < 3) {
        hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_1;
    } else if (food_stress > 80) {
        hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_5;
    } else if (food_stress > 60) {
        hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_4;
    } else if (food_stress > 40) {
        hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_3;
    } else {
        hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_2;
    }

    text_draw(translation_for(hunger_text), c->x_offset + 300, c->y_offset + 140, FONT_NORMAL_BLACK, 0);
    // food warnings
    if (m->mess_hall_max_morale_modifier < -20) {
        text_draw_centered(translation_for(TR_BUILDING_LEGION_FOOD_WARNING_2),
            c->x_offset + 20, c->y_offset + 340, c->width_blocks * 16 - 40, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    } else if (m->mess_hall_max_morale_modifier < -5) {
        text_draw_centered(translation_for(TR_BUILDING_LEGION_FOOD_WARNING_1),
            c->x_offset + 20, c->y_offset + 340, c->width_blocks * 16 - 40, FONT_NORMAL_BLACK, 0);
    } else if (m->mess_hall_max_morale_modifier > 0) {
        text_draw_centered(translation_for(TR_BUILDING_LEGION_FOOD_BONUS),
            c->x_offset + 20, c->y_offset + 340, c->width_blocks * 16 - 40, FONT_NORMAL_BLACK, 0);
    }

    if (m->num_figures) {
        // layout
        static const int OFFSETS_LEGIONARY[2][5] = {
            {0, 0, 2, 3, 4}, {0, 0, 3, 2, 4},
        };
        static const int OFFSETS_OTHER[2][5] = {
            {5, 6, 2, 3, 4}, {6, 5, 3, 2, 4},
        };
        const int *offsets;
        int index = 0;
        if (city_view_orientation() == DIR_6_LEFT || city_view_orientation() == DIR_2_RIGHT) {
            index = 1;
        }
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            offsets = OFFSETS_LEGIONARY[index];
        } else {
            offsets = OFFSETS_OTHER[index];
        }
        for (int i = 5 - c->formation_types; i < 5; i++) {
            image_draw(image_group(GROUP_FORT_FORMATIONS) + offsets[i], c->x_offset + 21 + 85 * i, c->y_offset + 181,
                COLOR_MASK_NONE, SCALE_NONE);
        }
        window_building_draw_legion_info_foreground(c);
    } else {
        // no soldiers
        int group_id;
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
        window_building_draw_description_at(c, 172, group_id, text_id);
    }
}

void window_building_draw_legion_info_foreground(building_info_context *c)
{
    const formation *m = formation_get(c->formation_id);
    if (!m->num_figures) {
        return;
    }
    for (int i = 5 - c->formation_types; i < 5; i++) {
        int has_focus = 0;
        if (data.focus_button_id) {
            if (data.focus_button_id - 1 == i) {
                has_focus = 1;
            }
        } else if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            if (i == 0 && m->layout == FORMATION_TORTOISE) {
                has_focus = 1;
            } else if (i == 1 && m->layout == FORMATION_COLUMN) {
                has_focus = 1;
            } else if (i == 2 && m->layout == FORMATION_DOUBLE_LINE_1) {
                has_focus = 1;
            } else if (i == 3 && m->layout == FORMATION_DOUBLE_LINE_2) {
                has_focus = 1;
            } else if (i == 4 && m->layout == FORMATION_MOP_UP) {
                has_focus = 1;
            }
        } else { // mounted/javelin
            if (i == 0 && m->layout == FORMATION_SINGLE_LINE_1) {
                has_focus = 1;
            } else if (i == 1 && m->layout == FORMATION_SINGLE_LINE_2) {
                has_focus = 1;
            } else if (i == 2 && m->layout == FORMATION_DOUBLE_LINE_1) {
                has_focus = 1;
            } else if (i == 3 && m->layout == FORMATION_DOUBLE_LINE_2) {
                has_focus = 1;
            } else if (i == 4 && m->layout == FORMATION_MOP_UP) {
                has_focus = 1;
            }
        }
        button_border_draw(c->x_offset + 19 + 85 * i, c->y_offset + 179, 84, 84, has_focus);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 270, c->width_blocks - 2, 4);

    int title_id;
    int text_id;
    switch (data.focus_button_id) {
        // single line or testudo
        case 1:
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                title_id = 12;
                text_id = m->has_military_training ? 18 : 17;
            } else {
                title_id = 16;
                text_id = 22;
            }
            break;
        case 2:
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                title_id = 13;
                text_id = m->has_military_training ? 19 : 17;
            } else {
                title_id = 16;
                text_id = 22;
            }
            break;
        case 3: // double line
        case 4:
            title_id = 14;
            text_id = 20;
            break;
        case 5: // mop up
            title_id = 15;
            text_id = 21;
            break;
        default:
            // no button selected: go for formation layout
            switch (m->layout) {
                case FORMATION_SINGLE_LINE_1:
                case FORMATION_SINGLE_LINE_2:
                    title_id = 16;
                    text_id = 22;
                    break;
                case FORMATION_DOUBLE_LINE_1:
                case FORMATION_DOUBLE_LINE_2:
                    title_id = 14;
                    text_id = 20;
                    break;
                case FORMATION_TORTOISE:
                    title_id = 12;
                    text_id = 18;
                    break;
                case FORMATION_MOP_UP:
                    title_id = 15;
                    text_id = 21;
                    break;
                case FORMATION_COLUMN:
                    title_id = 13;
                    text_id = 19;
                    break;
                default:
                    title_id = 16;
                    text_id = 22;
                    log_info("Unknown formation", 0, m->layout);
                    break;
            }
            break;
    }
    lang_text_draw(138, title_id, c->x_offset + 24, c->y_offset + 276, FONT_NORMAL_WHITE);
    lang_text_draw_multiline(138, text_id, c->x_offset + 24, c->y_offset + 292,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_GREEN);

    if (!m->is_at_fort && !m->in_distant_battle) {
        button_border_draw(c->x_offset + BLOCK_SIZE * (c->width_blocks - 18) / 2,
            c->y_offset + BLOCK_SIZE * c->height_blocks - 48, 288, 32, data.return_button_id == 1);
        lang_text_draw_centered(138, 58, c->x_offset + BLOCK_SIZE * (c->width_blocks - 18) / 2,
            c->y_offset + BLOCK_SIZE * c->height_blocks - 39, 288, FONT_NORMAL_BLACK);
    }
}

int window_building_handle_mouse_legion_info(const mouse *m, building_info_context *c)
{
    data.context_for_callback = c;
    int handled = generic_buttons_handle_mouse(m, c->x_offset, c->y_offset, layout_buttons, 5, &data.focus_button_id);
    if (formation_get(c->formation_id)->figure_type == FIGURE_FORT_LEGIONARY) {
        if (data.focus_button_id == 1 || (data.focus_button_id == 2 && c->formation_types == 3)) {
            data.focus_button_id = 0;
        }
    }
    if (!handled) {
        handled = generic_buttons_handle_mouse(m, c->x_offset + BLOCK_SIZE * (c->width_blocks - 18) / 2,
            c->y_offset + BLOCK_SIZE * c->height_blocks - 48, return_button, 1, &data.return_button_id);
    }
    data.context_for_callback = 0;
    return handled;
}

int window_building_get_legion_info_tooltip_text(building_info_context *c)
{
    return data.focus_button_id ? 147 : 0;
}

static void button_return_to_fort(int param1, int param2)
{
    formation *m = formation_get(data.context_for_callback->formation_id);
    if (!m->in_distant_battle && m->is_at_fort != 1) {
        formation_legion_return_home(m);
        window_city_show();
    }
}

static void button_layout(int index, int param2)
{
    formation *m = formation_get(data.context_for_callback->formation_id);
    if (m->in_distant_battle) {
        return;
    }
    if (index == 0 && data.context_for_callback->formation_types < 5) {
        return;
    }
    if (index == 1 && data.context_for_callback->formation_types < 4) {
        return;
    }
    // store layout in case of mop up
    int new_layout = m->layout;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        switch (index) {
            case 0: new_layout = FORMATION_TORTOISE; break;
            case 1: new_layout = FORMATION_COLUMN; break;
            case 2: new_layout = FORMATION_DOUBLE_LINE_1; break;
            case 3: new_layout = FORMATION_DOUBLE_LINE_2; break;
            case 4: new_layout = FORMATION_MOP_UP; break;
        }
    } else {
        switch (index) {
            case 0: new_layout = FORMATION_SINGLE_LINE_1; break;
            case 1: new_layout = FORMATION_SINGLE_LINE_2; break;
            case 2: new_layout = FORMATION_DOUBLE_LINE_1; break;
            case 3: new_layout = FORMATION_DOUBLE_LINE_2; break;
            case 4: new_layout = FORMATION_MOP_UP; break;
        }
    }
    formation_legion_change_layout(m, new_layout);
    switch (index) {
        case 0: sound_speech_play_file("wavs/cohort1.wav"); break;
        case 1: sound_speech_play_file("wavs/cohort2.wav"); break;
        case 2: sound_speech_play_file("wavs/cohort3.wav"); break;
        case 3: sound_speech_play_file("wavs/cohort4.wav"); break;
        case 4: sound_speech_play_file("wavs/cohort5.wav"); break;
    }
    window_city_military_show(data.context_for_callback->formation_id);
}

static void button_priority(int index, int param2)
{
    building *barracks = building_get(data.building_id);
    if (index != barracks->subtype.barracks_priority) {
        building_barracks_toggle_priority(barracks);
    }
}

void window_building_draw_watchtower(building_info_context *c)
{
    c->help_id = 85;
    window_building_play_sound(c, "wavs/tower2.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    text_draw_centered(translation_for(TR_BUILDING_WATCHTOWER),
        c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else {
        building *b = building_get(c->building_id);
        if (!b->figure_id4) {
            text_draw_multiline(translation_for(TR_BUILDING_WATCHTOWER_DESC_NO_SOLDIERS),
                c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        } else {
            text_draw_multiline(translation_for(TR_BUILDING_WATCHTOWER_DESC),
                c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_palisade(building_info_context *c)
{
    c->help_id = 85;
    window_building_play_sound(c, "wavs/wall.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    text_draw_centered(translation_for(TR_BUILDING_PALISADE), c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK, 0);
    window_building_draw_description_at(c, BLOCK_SIZE * c->height_blocks - 158, CUSTOM_TRANSLATION,
        TR_BUILDING_PALISADE_DESC);
}

