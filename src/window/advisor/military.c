#include "military.h"

#include "city/data_private.h"
#include "city/figures.h"
#include "city/military.h"
#include "city/view.h"
#include "core/calc.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "scenario/invasion.h"
#include "translation/translation.h"
#include "window/city.h"

#define ADVISOR_HEIGHT 27

#define MAX_BUTTONS 18

#define MAX_VISIBLE_LEGIONS 6

static void button_go_to_legion(int legion_id, int param2);
static void button_return_to_fort(int legion_id, int param2);
static void button_empire_service(int legion_id, int param2);
static void button_return_all_to_fort(int param1, int param2);
static void on_scroll(void);

static scrollbar_type scrollbar = { 592, 70, 272, 576, MAX_VISIBLE_LEGIONS, on_scroll };

static generic_button fort_buttons[] = {
    {384, 83, 30, 30, button_go_to_legion, button_none, 1, 0},
    {464, 83, 30, 30, button_return_to_fort, button_none, 1, 0},
    {544, 83, 30, 30, button_empire_service, button_none, 1, 0},
    {384, 127, 30, 30, button_go_to_legion, button_none, 2, 0},
    {464, 127, 30, 30, button_return_to_fort, button_none, 2, 0},
    {544, 127, 30, 30, button_empire_service, button_none, 2, 0},
    {384, 171, 30, 30, button_go_to_legion, button_none, 3, 0},
    {464, 171, 30, 30, button_return_to_fort, button_none, 3, 0},
    {544, 171, 30, 30, button_empire_service, button_none, 3, 0},
    {384, 215, 30, 30, button_go_to_legion, button_none, 4, 0},
    {464, 215, 30, 30, button_return_to_fort, button_none, 4, 0},
    {544, 215, 30, 30, button_empire_service, button_none, 4, 0},
    {384, 259, 30, 30, button_go_to_legion, button_none, 5, 0},
    {464, 259, 30, 30, button_return_to_fort, button_none, 5, 0},
    {544, 259, 30, 30, button_empire_service, button_none, 5, 0},
    {384, 303, 30, 30, button_go_to_legion, button_none, 6, 0},
    {464, 303, 30, 30, button_return_to_fort, button_none, 6, 0},
    {544, 303, 30, 30, button_empire_service, button_none, 6, 0},
};

static generic_button additional_buttons[] = {
    {445, 28, 60, 40, button_return_all_to_fort, button_none, 0, 0}
};

static int focus_button_id;
static int focus_additional_button_id;
static int num_legions;

static void init(void)
{
    num_legions = formation_get_num_legions();
    scrollbar_init(&scrollbar, 0, num_legions);
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 1, 10, 10, COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw(51, 0, 60, 12, FONT_LARGE_BLACK);

    lang_text_draw(51, 1, 374, 35, FONT_SMALL_PLAIN);
    lang_text_draw(51, 2, 374, 50, FONT_SMALL_PLAIN);
    lang_text_draw(51, 3, 454, 35, FONT_SMALL_PLAIN);
    lang_text_draw(51, 4, 454, 50, FONT_SMALL_PLAIN);
    lang_text_draw(51, 5, 534, 35, FONT_SMALL_PLAIN);
    lang_text_draw(51, 6, 534, 50, FONT_SMALL_PLAIN);
    lang_text_draw(138, 36, 274, 50, FONT_SMALL_PLAIN);

    int enemy_text_id;
    if (city_figures_enemies()) {
        enemy_text_id = 10;
    } else if (city_figures_imperial_soldiers()) {
        enemy_text_id = 11;
    } else if (scenario_invasion_exists_upcoming()) {
        enemy_text_id = 9;
    } else {
        enemy_text_id = 8;
    }
    int distant_battle_text_id;
    if (city_military_distant_battle_roman_army_is_traveling_back()) {
        distant_battle_text_id = 15;
    } else if (city_military_distant_battle_roman_army_is_traveling_forth()) {
        distant_battle_text_id = 14;
    } else if (city_military_months_until_distant_battle() > 0) {
        distant_battle_text_id = 13;
    } else {
        distant_battle_text_id = 12;
    }
    int bullet_x = 60;
    int text_x = 80;
    int food_text;
    int food_stress = city_data.mess_hall.food_stress_cumulative;

    if (food_stress > 50 && !city_mess_hall_months_food_stored()) {
        food_text = TR_ADVISOR_LEGION_FOOD_CRITICAL;
    }
    else if (food_stress > 20 && !city_mess_hall_months_food_stored()) {
        food_text = TR_ADVISOR_LEGION_FOOD_NEEDED;
    }
    else {
        food_text = TR_ADVISOR_LEGION_MONTHS_FOOD_STORED;
    }

    if (num_legions <= 0) {
        image_draw(image_group(GROUP_BULLET), bullet_x, 359, COLOR_MASK_NONE, SCALE_NONE);
        lang_text_draw(51, enemy_text_id, text_x, 358, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 379, COLOR_MASK_NONE, SCALE_NONE);
        lang_text_draw(51, distant_battle_text_id, text_x, 378, FONT_NORMAL_BLACK);
    } else {
        // has forts
        image_draw(image_group(GROUP_BULLET), bullet_x, 349, COLOR_MASK_NONE, SCALE_NONE);
        int width = lang_text_draw_amount(8, 46, city_military_total_soldiers(), text_x - 5, 348, FONT_NORMAL_BLACK);
        width += lang_text_draw(51, 7, text_x + width, 348, FONT_NORMAL_BLACK);
        lang_text_draw_amount(8, 48, city_military_total_legions(), text_x + width, 348, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 369, COLOR_MASK_NONE, SCALE_NONE);
        lang_text_draw(51, enemy_text_id, text_x, 368, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 389, COLOR_MASK_NONE, SCALE_NONE);
        lang_text_draw(51, distant_battle_text_id, text_x, 388, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 409, COLOR_MASK_NONE, SCALE_NONE);
        width = text_draw(translation_for(food_text), text_x, 409, FONT_NORMAL_BLACK, 0);
        if (food_text == TR_ADVISOR_LEGION_MONTHS_FOOD_STORED) {
            text_draw_number(city_mess_hall_months_food_stored(), '@', " ", text_x + width, 409, FONT_NORMAL_BLACK, 0);
        }
    }

    inner_panel_draw(16, 70, 36, 17);
    if (num_legions <= 0) {
        lang_text_draw_multiline(51, 16, 64, 200, 496, FONT_NORMAL_GREEN);
        return ADVISOR_HEIGHT;
    }
    for (int i = 0; i < 6 && i < num_legions; i++) {
        const formation *m = formation_get(formation_for_legion(i + 1 + scrollbar.scroll_position));
        button_border_draw(22, 77 + 44 * i, 560, 40, 0);
        image_draw(image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id, 32, 82 + 44 * i,
            COLOR_MASK_NONE, SCALE_NONE);
        lang_text_draw(138, m->legion_id, 84, 83 + 44 * i, FONT_NORMAL_WHITE);
        int width = text_draw_number(m->num_figures, '@', " ", 84, 100 + 44 * i, FONT_NORMAL_GREEN, 0);
        switch (m->figure_type) {
            case FIGURE_FORT_LEGIONARY:
                lang_text_draw(138, 33, 84 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
            case FIGURE_FORT_MOUNTED:
                lang_text_draw(138, 34, 84 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
            case FIGURE_FORT_JAVELIN:
                lang_text_draw(138, 35, 84 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
        }
        int morale_offset = m->morale / 5;
        if (morale_offset > 20) {
            morale_offset = 20;
        }
        lang_text_draw_centered(138, 37 + morale_offset, 224, 91 + 44 * i, 150, FONT_NORMAL_GREEN);

        int image_id = image_group(GROUP_FORT_ICONS);
        button_border_draw(384, 83 + 44 * i, 30, 30, 0);
        image_draw(image_id, 387, 86 + 44 * i, COLOR_MASK_NONE, SCALE_NONE);

        button_border_draw(464, 83 + 44 * i, 30, 30, 0);
        if (m->is_at_fort || m->in_distant_battle) {
            image_draw(image_id + 2, 467, 86 + 44 * i, COLOR_MASK_NONE, SCALE_NONE);
        } else {
            image_draw(image_id + 1, 467, 86 + 44 * i, COLOR_MASK_NONE, SCALE_NONE);
        }

        button_border_draw(544, 83 + 44 * i, 30, 30, 0);
        if (m->empire_service) {
            image_draw(image_id + 3, 547, 86 + 44 * i, COLOR_MASK_NONE, SCALE_NONE);
        } else {
            image_draw(image_id + 4, 547, 86 + 44 * i, COLOR_MASK_NONE, SCALE_NONE);
        }
    }

    return ADVISOR_HEIGHT;
}

static int get_num_legions_not_at_fort(void)
{
    int num_legions_not_at_fort = 0;
    for (int i = 0; i < num_legions; i++) {
        const formation* m = formation_get(formation_for_legion(i + 1));
        if (!m->in_distant_battle && !m->is_at_fort) {
            num_legions_not_at_fort++;
        }
    }
    return num_legions_not_at_fort;
}

static void draw_foreground(void)
{
    scrollbar_draw(&scrollbar);
    num_legions = formation_get_num_legions();
    for (int i = 0; i < 6 && i < num_legions; i++) {
        button_border_draw(384, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 1);
        button_border_draw(464, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 2);
        button_border_draw(544, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 3);
    }

    int num_legions_not_at_fort = get_num_legions_not_at_fort();
    if (num_legions_not_at_fort > 0) {
        button_border_draw(445, 28, 60, 40, focus_additional_button_id == 1);
    }
}

static int handle_mouse(const mouse *m)
{
    focus_additional_button_id = 0;
    if (scrollbar_handle_mouse(&scrollbar, m, 1)) {
        focus_button_id = 0;
        return 1;
    }
    int buttons = 3 * num_legions;
    if (buttons > MAX_BUTTONS) {
        buttons = MAX_BUTTONS;
    }
    int result = generic_buttons_handle_mouse(m, 0, 0, fort_buttons, buttons, &focus_button_id);
    if (result == 0) {
        int num_legions_not_at_fort = get_num_legions_not_at_fort();
        if (num_legions_not_at_fort > 0) {
            result = generic_buttons_handle_mouse(m, 0, 0, additional_buttons, 1, &focus_additional_button_id);
        }
    }
    return result;
}

static void button_go_to_legion(int legion_id, int param2)
{
    const formation *m = formation_get(formation_for_legion(legion_id+scrollbar.scroll_position));
    city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
    window_city_show();
}

static void return_legion_to_fort(int legion_id) 
{
    formation *m = formation_get(formation_for_legion(legion_id));
    if (!m->in_distant_battle && !m->is_at_fort) {
        formation_legion_return_home(m);
        window_invalidate();
    }
}

static void button_return_to_fort(int legion_id, int param2)
{
    return_legion_to_fort(legion_id + scrollbar.scroll_position);
}

static void button_empire_service(int legion_id, int param2)
{
    int formation_id = formation_for_legion(legion_id + scrollbar.scroll_position);
    formation_toggle_empire_service(formation_id);
    formation_calculate_figures();
    window_invalidate();
}

static void button_return_all_to_fort(int param1, int param2)
{
    int num_legions_not_at_fort = get_num_legions_not_at_fort();
    if (num_legions_not_at_fort > 0) {
        for (int i = 0; i < num_legions; i++) {
            return_legion_to_fort(i + 1);
        }
    }
}

static void on_scroll(void)
{
    window_invalidate();
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    if (focus_additional_button_id) {
        r->translation_key = TR_RETURN_ALL_TO_FORT;
    }
}

const advisor_window_type *window_advisor_military(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    init();
    return &window;
}
