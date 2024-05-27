#include "military.h"

#include "assets/assets.h"
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
static void button_delivery(int index, int param2);

static generic_button layout_buttons[] = {
    {19, 179, 84, 84, button_layout, button_none, 0, 0},
    {104, 179, 84, 84, button_layout, button_none, 1, 0},
    {189, 179, 84, 84, button_layout, button_none, 2, 0},
    {274, 179, 84, 84, button_layout, button_none, 3, 0},
    {359, 179, 84, 84, button_layout, button_none, 4, 0}
};

static generic_button priority_buttons[] = {
    {0, 0, 40, 40, button_priority, button_none, 0, 0},
    {56, 0, 40, 40, button_priority, button_none, 1, 0},
    {112, 0, 40, 40, button_priority, button_none, 2, 0},
    {168, 0, 40, 40, button_priority, button_none, 3, 0},
    {224, 0, 40, 40, button_priority, button_none, 4, 0},
    {280, 0, 40, 40, button_priority, button_none, 5, 0},
    {336, 0, 40, 40, button_priority, button_none, 6, 0},
};

static generic_button delivery_buttons[] = {
    {0, 0, 52, 52, button_delivery, button_none, 0, 0},
};

static generic_button return_button[] = {
    {0, 0, 288, 32, button_return_to_fort, button_none, 0, 0},
};

static struct {
    unsigned int focus_button_id;
    unsigned int focus_priority_button_id;
    unsigned int focus_delivery_button_id;
    unsigned int return_button_id;
    int building_id;
    building_info_context *context_for_callback;
} data;

static void draw_priority_buttons(int x, int y, unsigned int buttons, int building_id)
{
    int base_priority_image_id = assets_get_image_id("UI", "Barracks_Priority_Legionaries_OFF");
    data.building_id = building_id;    

    for (unsigned int i = 0; i < buttons; i++) {
        int has_focus = 0;
        if (data.focus_priority_button_id) {
            if (data.focus_priority_button_id - 1 == i) {
                has_focus = 1;
            }
        }
        int x_adj = x + priority_buttons[i].x;
        int y_adj = y + priority_buttons[i].y;

        building *barracks = building_get(data.building_id);
        unsigned int priority = building_barracks_get_priority(barracks);

        if (has_focus || priority == i) {
            button_border_draw(x_adj - 3, y_adj - 3, 46, 46, 1);
        }
        image_draw(base_priority_image_id + i * 2 + (i == priority ? 1 : 0), x_adj, y_adj, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_delivery_buttons(int x, int y, int building_id)
{    
    data.building_id = building_id;

    building *barracks = building_get(data.building_id);

    int accept_delivery = barracks->accepted_goods[RESOURCE_WEAPONS];
    
    if (!accept_delivery) {
        inner_panel_draw(x + 2, y + 2, 3, 3);
    }

    image_draw(image_group(GROUP_FIGURE_CARTPUSHER_CART) + 104, x + 7, y + 7, COLOR_MASK_NONE, SCALE_NONE);

    if (!accept_delivery) {
        image_draw(assets_get_image_id("UI", "Large_Widget_Cross"), x + 15, y + 15,
        COLOR_MASK_NONE, SCALE_NONE);
    }
    
    button_border_draw(x, y, 52, 52, data.focus_delivery_button_id || !accept_delivery ? 1 : 0);
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
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
}

void window_building_draw_barracks(building_info_context *c)
{
    c->help_id = 37;
    data.building_id = c->building_id;
    window_building_play_sound(c, "wavs/barracks.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(136, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    image_draw(resource_get_data(RESOURCE_WEAPONS)->image.icon, c->x_offset + 32, c->y_offset + 60,
        COLOR_MASK_NONE, SCALE_NONE);

    building *b = building_get(c->building_id);
    if (b->resources[RESOURCE_WEAPONS] < 1) {
        lang_text_draw_amount(8, 10, 0, c->x_offset + 60, c->y_offset + 66, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount(8, 10, b->resources[RESOURCE_WEAPONS], c->x_offset + 60, c->y_offset + 66, FONT_NORMAL_BLACK);
    }

    if (!c->has_road_access) {
        window_building_draw_description_at(c, 106, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description_at(c, 106, 136, 3);
    } else if (!c->barracks_soldiers_requested) {
        window_building_draw_description_at(c, 106, 136, 4);
    } else {
        int offset = 0;
        if (b->resources[RESOURCE_WEAPONS] > 0) {
            offset = 4;
        }
        if (city_data.mess_hall.food_stress_cumulative > 50) {
            text_draw_multiline(translation_for(TR_BUILDING_BARRACKS_FOOD_WARNING_2),
                c->x_offset + 32, c->y_offset + 106, 16 * c->width_blocks - 30, 0, FONT_NORMAL_BLACK, 0);
        } else if (city_data.mess_hall.food_stress_cumulative > 20) {
            text_draw_multiline(translation_for(TR_BUILDING_BARRACKS_FOOD_WARNING),
                c->x_offset + 32, c->y_offset + 106, 16 * c->width_blocks - 30, 0, FONT_NORMAL_BLACK, 0);
        } else if (c->worker_percentage >= 100) {
            window_building_draw_description_at(c, 106, 136, 5 + offset);
        } else if (c->worker_percentage >= 66) {
            window_building_draw_description_at(c, 106, 136, 6 + offset);
        } else if (c->worker_percentage >= 33) {
            window_building_draw_description_at(c, 106, 136, 7 + offset);
        } else {
            window_building_draw_description_at(c, 106, 136, 8 + offset);
        }
    }

    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_BARRACKS_PRIORITY,
        c->x_offset + 32, c->y_offset + 170, FONT_NORMAL_BLACK); // "Priority"
    
    inner_panel_draw(c->x_offset + 16, c->y_offset + 290, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 294);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 298);
}

void window_building_draw_barracks_foreground(building_info_context *c)
{
    inner_panel_draw(c->x_offset + 16, c->y_offset + 188, c->width_blocks - 2, 5);
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_BARRACKS_FORTS,
        c->x_offset + 42, c->y_offset + 200, FONT_NORMAL_BROWN); // "Forts"
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_BARRACKS_TOWERS,
        c->x_offset + 324, c->y_offset + 200, FONT_NORMAL_BROWN); // "Towers"

    draw_priority_buttons(c->x_offset + 42, c->y_offset + 218, 7, data.building_id);
    draw_delivery_buttons(c->x_offset + 392, c->y_offset + 40, data.building_id);
}

void window_building_draw_priority_buttons(int x, int y, int building_id)
{
    draw_priority_buttons(x, y, 7, building_id);
}

void window_building_draw_delivery_buttons(int x, int y, int building_id)
{
    draw_delivery_buttons(x, y, building_id);
}

int window_building_handle_mouse_barracks(const mouse *m, building_info_context *c)
{
    if (generic_buttons_handle_mouse(m, c->x_offset + 46, c->y_offset + 222,
        priority_buttons, 7, &data.focus_priority_button_id) || 
        generic_buttons_handle_mouse(m, c->x_offset + 392, c->y_offset + 40,
        delivery_buttons, 1, &data.focus_delivery_button_id)) {
        window_invalidate();
        return 1;
    }

    return 0;
}

int window_building_handle_mouse_grand_temple_mars(const mouse *m, building_info_context *c)
{
    if (generic_buttons_handle_mouse(m, c->x_offset + 50, c->y_offset + 135,
        priority_buttons, 7, &data.focus_priority_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 408, c->y_offset + 40,
        delivery_buttons, 1, &data.focus_delivery_button_id)
        ) {
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
    } else if (c->worker_percentage < 100) {
        window_building_draw_description(c, 135, 1);
    } else {
        window_building_draw_description(c, 135, 3);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
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
        window_building_draw_levy(building_get_levy(b), c->x_offset + 300, c->y_offset + 150);
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
    if (m->figure_type == FIGURE_FORT_INFANTRY) {
        if (m->is_halted) {
            flag_image_id = assets_get_image_id("UI", "auxinf_banner_0");
        } else {
            flag_image_id = assets_get_image_id("UI", "auxinf_banner_01");
        }
    }
    if (m->figure_type == FIGURE_FORT_ARCHER) {
        if (m->is_halted) {
            flag_image_id = assets_get_image_id("UI", "auxarch_banner_0");
        } else {
            flag_image_id = assets_get_image_id("UI", "auxarch_banner_01");
        }
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
            c->x_offset + 20, c->y_offset + 360, c->width_blocks * 16 - 40, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    } else if (m->mess_hall_max_morale_modifier < -5) {
        text_draw_centered(translation_for(TR_BUILDING_LEGION_FOOD_WARNING_1),
            c->x_offset + 20, c->y_offset + 360, c->width_blocks * 16 - 40, FONT_NORMAL_BLACK, 0);
    } else if (m->mess_hall_max_morale_modifier > 0) {
        text_draw_centered(translation_for(TR_BUILDING_LEGION_FOOD_BONUS),
            c->x_offset + 20, c->y_offset + 360, c->width_blocks * 16 - 40, FONT_NORMAL_BLACK, 0);
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
        if (m->figure_type == FIGURE_FORT_LEGIONARY || m->figure_type == FIGURE_FORT_INFANTRY) {
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
    for (unsigned int i = 5 - c->formation_types; i < 5; i++) {
        int has_focus = 0;
        if (data.focus_button_id) {
            if (data.focus_button_id - 1 == i) {
                has_focus = 1;
            }
        } else if (m->figure_type == FIGURE_FORT_LEGIONARY || m->figure_type == FIGURE_FORT_INFANTRY) {
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

    inner_panel_draw(c->x_offset + 16, c->y_offset + 270, c->width_blocks - 2, 5);

    int title_id;
    int text_id;
    switch (data.focus_button_id) {
        // single line or testudo
        case 1:
            if (m->figure_type == FIGURE_FORT_LEGIONARY || m->figure_type == FIGURE_FORT_INFANTRY) {
                title_id = 12;
                text_id = m->has_military_training ? 18 : 17;
            } else {
                title_id = 16;
                text_id = 22;
            }
            break;
        case 2:
            if (m->figure_type == FIGURE_FORT_LEGIONARY || m->figure_type == FIGURE_FORT_INFANTRY) {
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
    int figure_type = formation_get(c->formation_id)->figure_type;
    if (figure_type == FIGURE_FORT_LEGIONARY || figure_type == FIGURE_FORT_INFANTRY) {
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

void window_building_barracks_get_tooltip_priority(int *translation)
{
    switch (data.focus_priority_button_id) {
        case 1:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_FORT;
            break;
        case 2:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_JAVELIN;
            break;
        case 3:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_MOUNTED;
            break;
        case 4:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_AUXINF;
            break;
        case 5:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_AUXARCH;
            break;
        case 6:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_TOWER;
            break;
        case 7:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_WATCHTOWER;
            break;
        case 8:
            *translation = TR_TOOLTIP_BARRACKS_PRIORITY_FORT;
            break;
        default:
            break;
    }

    if (data.focus_delivery_button_id) {
        building *barracks = building_get(data.building_id);
        if (barracks->accepted_goods[RESOURCE_WEAPONS]) {
            *translation = TR_TOOLTIP_BUTTON_REJECT_DELIVERY;
        } else {
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_DELIVERY;
        }
    }
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
    if (m->figure_type == FIGURE_FORT_LEGIONARY || m->figure_type == FIGURE_FORT_INFANTRY) {
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
    building_barracks_set_priority(barracks, index);    
}

static void button_delivery(int index, int param2)
{
    building *barracks = building_get(data.building_id);
    building_barracks_toggle_delivery(barracks);    
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
                c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), 0, FONT_NORMAL_BLACK, 0);
        } else {
            text_draw_multiline(translation_for(TR_BUILDING_WATCHTOWER_DESC),
                c->x_offset + 32, c->y_offset + 76, 16 * (c->width_blocks - 4), 0, FONT_NORMAL_BLACK, 0);
        }
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
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

void window_building_draw_armoury(building_info_context *c)
{
    c->help_id = 85;
    building *b = building_get(c->building_id);

    window_building_play_sound(c, "wavs/tower3.wav");

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    text_draw_centered(translation_for(TR_BUILDING_ARMOURY), c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK, 0);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_ARMOURY_NO_EMPLOYEES);
    } else if (c->worker_percentage <= 50) {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_ARMOURY_SOME_EMPLOYEES);
    } else if (c->worker_percentage < 100) {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_ARMOURY_MANY_EMPLOYEES);
    } else {
        window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_ARMOURY_DESC);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    window_building_draw_risks(c, c->x_offset + c->width_blocks * BLOCK_SIZE - 76, c->y_offset + 144);
}
