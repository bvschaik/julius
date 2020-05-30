#include "military.h"

#include "city/figures.h"
#include "city/military.h"
#include "city/view.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "scenario/invasion.h"
#include "window/city.h"

#define ADVISOR_HEIGHT 26

#define SCROLL_BUTTON_HEIGHT 26
#define SCROLL_BUTTON_WIDTH 39
#define SCROLL_DOT_SIZE 25
#define TOTAL_BUTTON_HEIGHT (2 * SCROLL_BUTTON_HEIGHT + SCROLL_DOT_SIZE)

static struct {
    int button_delta;
} data;



static void text_scroll(int is_down, int num_lines)
{
    if (is_down) {
        data.button_delta += 1;
    }
    else {
        data.button_delta -= 1;
    }
    if (data.button_delta < 0) {
        data.button_delta = 0;
    }
    int max_delta = formation_get_num_legions() - 6;
    if (data.button_delta > max_delta) {
        data.button_delta = max_delta;
    }
    if (data.button_delta < 0) {
        data.button_delta = 0;
    }



}

static void init()
{
    data.button_delta = 0;
}



static image_button image_button_scroll_up = {
    0, 0, SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT, IB_NORMAL,
    GROUP_OK_CANCEL_SCROLL_BUTTONS, 8, text_scroll, button_none, 0, 1, 1
};
static image_button image_button_scroll_down = {
    0, 0, SCROLL_BUTTON_WIDTH, SCROLL_BUTTON_HEIGHT, IB_NORMAL,
    GROUP_OK_CANCEL_SCROLL_BUTTONS, 12, text_scroll, button_none, 1, 1, 1
};

static void button_go_to_legion(int legion_id, int param2);
static void button_return_to_fort(int legion_id, int param2);
static void button_empire_service(int legion_id, int param2);

static generic_button fort_buttons[] = {
    {400, 83, 30, 30, button_go_to_legion, button_none, 1, 0},
    {480, 83, 30, 30, button_return_to_fort, button_none, 1, 0},
    {560, 83, 30, 30, button_empire_service, button_none, 1, 0},
    {400, 127, 30, 30, button_go_to_legion, button_none, 2, 0},
    {480, 127, 30, 30, button_return_to_fort, button_none, 2, 0},
    {560, 127, 30, 30, button_empire_service, button_none, 2, 0},
    {400, 171, 30, 30, button_go_to_legion, button_none, 3, 0},
    {480, 171, 30, 30, button_return_to_fort, button_none, 3, 0},
    {560, 171, 30, 30, button_empire_service, button_none, 3, 0},
    {400, 215, 30, 30, button_go_to_legion, button_none, 4, 0},
    {480, 215, 30, 30, button_return_to_fort, button_none, 4, 0},
    {560, 215, 30, 30, button_empire_service, button_none, 4, 0},
    {400, 259, 30, 30, button_go_to_legion, button_none, 5, 0},
    {480, 259, 30, 30, button_return_to_fort, button_none, 5, 0},
    {560, 259, 30, 30, button_empire_service, button_none, 5, 0},
    {400, 303, 30, 30, button_go_to_legion, button_none, 6, 0},
    {480, 303, 30, 30, button_return_to_fort, button_none, 6, 0},
    {560, 303, 30, 30, button_empire_service, button_none, 6, 0},
};

static int focus_button_id;
static int num_legions;

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 1, 10, 10);
    lang_text_draw(51, 0, 60, 12, FONT_LARGE_BLACK);

    lang_text_draw(51, 1, 390, 43, FONT_SMALL_PLAIN);
    lang_text_draw(51, 2, 390, 58, FONT_SMALL_PLAIN);
    lang_text_draw(51, 3, 470, 43, FONT_SMALL_PLAIN);
    lang_text_draw(51, 4, 470, 58, FONT_SMALL_PLAIN);
    lang_text_draw(51, 5, 550, 43, FONT_SMALL_PLAIN);
    lang_text_draw(51, 6, 550, 58, FONT_SMALL_PLAIN);
    lang_text_draw(138, 36, 290, 58, FONT_SMALL_PLAIN);

    num_legions = formation_get_num_legions();

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
    if (num_legions <= 0) {
        image_draw(image_group(GROUP_BULLET), bullet_x, 359);
        lang_text_draw(51, enemy_text_id, text_x, 358, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 379);
        lang_text_draw(51, distant_battle_text_id, text_x, 378, FONT_NORMAL_BLACK);
    } else {
        // has forts
        image_draw(image_group(GROUP_BULLET), bullet_x, 349);
        int width = lang_text_draw_amount(8, 46, city_military_total_soldiers(), text_x, 348, FONT_NORMAL_BLACK);
        width += lang_text_draw(51, 7, text_x + width, 348, FONT_NORMAL_BLACK);
        lang_text_draw_amount(8, 48, city_military_total_legions(), text_x + width, 348, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 369);
        lang_text_draw(51, enemy_text_id, text_x, 368, FONT_NORMAL_BLACK);

        image_draw(image_group(GROUP_BULLET), bullet_x, 389);
        lang_text_draw(51, distant_battle_text_id, text_x, 388, FONT_NORMAL_BLACK);
    }

    inner_panel_draw(32, 70, 36, 17);
    if (num_legions <= 0) {
        lang_text_draw_multiline(51, 16, 64, 200, 496, FONT_NORMAL_GREEN);
        return ADVISOR_HEIGHT;
    }
    for (int i = 0; i < 6 && i < num_legions; i++) {
        const formation *m = formation_get(formation_for_legion(i + 1 + data.button_delta));
        button_border_draw(38, 77 + 44 * i, 560, 40, 0);
        image_draw(image_group(GROUP_FIGURE_FORT_STANDARD_ICONS) + m->legion_id, 48, 82 + 44 * i);
        lang_text_draw(138, m->legion_id, 100, 83 + 44 * i, FONT_NORMAL_WHITE);
        int width = text_draw_number(m->num_figures, '@', " ", 100, 100 + 44 * i, FONT_NORMAL_GREEN);
        switch (m->figure_type) {
            case FIGURE_FORT_LEGIONARY:
                lang_text_draw(138, 33, 100 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
            case FIGURE_FORT_MOUNTED:
                lang_text_draw(138, 34, 100 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
            case FIGURE_FORT_JAVELIN:
                lang_text_draw(138, 35, 100 + width, 100 + 44 * i, FONT_NORMAL_GREEN);
                break;
        }
        lang_text_draw_centered(138, 37 + m->morale / 5, 240, 91 + 44 * i, 150, FONT_NORMAL_GREEN);

        int image_id = image_group(GROUP_FORT_ICONS);
        button_border_draw(400, 83 + 44 * i, 30, 30, 0);
        image_draw(image_id, 403, 86 + 44 * i);

        button_border_draw(480, 83 + 44 * i, 30, 30, 0);
        if (m->is_at_fort) {
            image_draw(image_id + 2, 483, 86 + 44 * i);
        } else {
            image_draw(image_id + 1, 483, 86 + 44 * i);
        }

        button_border_draw(560, 83 + 44 * i, 30, 30, 0);
        if (m->empire_service) {
            image_draw(image_id + 3, 563, 86 + 44 * i);
        } else {
            image_draw(image_id + 4, 563, 86 + 44 * i);
        }
    }

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{

    num_legions = formation_get_num_legions();
    for (int i = 0; i < 6 && i < num_legions; i++) {
        button_border_draw(400, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 1);
        button_border_draw(480, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 2);
        button_border_draw(560, 83 + 44 * i, 30, 30, focus_button_id == 3 * i + 3);
    }
    image_buttons_draw(590, 10, &image_button_scroll_up, 1);
    image_buttons_draw(590, 380, &image_button_scroll_down, 1);
}

static int handle_mouse(const mouse *m)
{
    if (image_buttons_handle_mouse(m, 590, 10, &image_button_scroll_up, 1, 0)) {
        window_invalidate();
        return 1;
    }
    if (image_buttons_handle_mouse(m, 590, 380, &image_button_scroll_down, 1, 0)) {
        window_invalidate();
        return 1;
    }
    return generic_buttons_handle_mouse(m, 0, 0, fort_buttons, 3 * num_legions, &focus_button_id);
}

static void button_go_to_legion(int legion_id, int param2)
{
    const formation *m = formation_get(formation_for_legion(legion_id+data.button_delta));
    city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
    window_city_show();
}

static void button_return_to_fort(int legion_id, int param2)
{
    formation *m = formation_get(formation_for_legion(legion_id+data.button_delta));
    if (!m->in_distant_battle) {
        formation_legion_return_home(m);
        window_invalidate();
    }
}

static void button_empire_service(int legion_id, int param2)
{
    int formation_id = formation_for_legion(legion_id+data.button_delta);
    formation_toggle_empire_service(formation_id);
    formation_calculate_figures();
    window_invalidate();
}

const advisor_window_type *window_advisor_military(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    init();
    return &window;
}
