#include "ratings.h"

#include "city/ratings.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/lang.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

#define ADVISOR_HEIGHT 27

static void button_rating(int rating, int param2);

static generic_button rating_buttons[] = {
    { 80, 286, 110, 66, button_rating, button_none, SELECTED_RATING_CULTURE, 0},
    {200, 286, 110, 66, button_rating, button_none, SELECTED_RATING_PROSPERITY, 0},
    {320, 286, 110, 66, button_rating, button_none, SELECTED_RATING_PEACE, 0},
    {440, 286, 110, 66, button_rating, button_none, SELECTED_RATING_FAVOR, 0},
};

static int focus_button_id;

void draw_rating_column(int x_offset, int y_offset, int value, int has_reached)
{
    int image_base = image_group(GROUP_RATINGS_COLUMN);
    int y = y_offset - image_get(image_base)->height;
    int value_to_draw = value;
    if (has_reached && value < 25) {
        value_to_draw = 25;
    }

    image_draw(image_base, x_offset, y, COLOR_MASK_NONE, SCALE_NONE);
    for (int i = 0; i < 2 * value_to_draw; i++) {
        image_draw(image_base + 1, x_offset + 11, --y, COLOR_MASK_NONE, SCALE_NONE);
    }
    if (has_reached) {
        image_draw(image_base + 2, x_offset - 6, y, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 3, 10, 10, COLOR_MASK_NONE, SCALE_NONE);
    int width = lang_text_draw(53, 0, 60, 12, FONT_LARGE_BLACK);
    if (!scenario_criteria_population_enabled() || scenario_is_open_play()) {
        lang_text_draw(53, 7, 80 + width, 17, FONT_NORMAL_BLACK);
    } else {
        width += lang_text_draw(53, 6, 80 + width, 17, FONT_NORMAL_BLACK);
        text_draw_number(scenario_criteria_population(), '@', ")", 80 + width, 17, FONT_NORMAL_BLACK, 0);
    }

    image_draw(image_group(GROUP_RATINGS_BACKGROUND), 60, 48, COLOR_MASK_NONE, SCALE_NONE);

    int open_play = scenario_is_open_play();

    // culture
    int culture = city_rating_culture();
    int has_culture_goal = !open_play && scenario_criteria_culture_enabled();
    button_border_draw(80, 286, 110, 66, focus_button_id == SELECTED_RATING_CULTURE);
    lang_text_draw_centered(53, 1, 80, 294, 110, FONT_NORMAL_BLACK);
    text_draw_number_centered(culture, 80, 309, 100, FONT_LARGE_BLACK);
    width = text_draw_number(has_culture_goal ? scenario_criteria_culture() : 0,
        '@', " ", 85, 334, FONT_NORMAL_BLACK, 0);
    lang_text_draw(53, 5, 85 + width, 334, FONT_NORMAL_BLACK);
    int has_reached = !has_culture_goal || culture >= scenario_criteria_culture();
    draw_rating_column(110, 274, culture, has_reached);

    // prosperity
    int prosperity = city_rating_prosperity();
    int has_prosperity_goal = !open_play && scenario_criteria_prosperity_enabled();
    button_border_draw(200, 286, 110, 66, focus_button_id == SELECTED_RATING_PROSPERITY);
    lang_text_draw_centered(53, 2, 200, 294, 110, FONT_NORMAL_BLACK);
    text_draw_number_centered(prosperity, 200, 309, 100, FONT_LARGE_BLACK);
    width = text_draw_number(has_prosperity_goal ? scenario_criteria_prosperity() : 0,
        '@', " ", 205, 334, FONT_NORMAL_BLACK, 0);
    lang_text_draw(53, 5, 205 + width, 334, FONT_NORMAL_BLACK);
    has_reached = !has_prosperity_goal || prosperity >= scenario_criteria_prosperity();
    draw_rating_column(230, 274, prosperity, has_reached);

    // peace
    int peace = city_rating_peace();
    int has_peace_goal = !open_play && scenario_criteria_peace_enabled();
    button_border_draw(320, 286, 110, 66, focus_button_id == SELECTED_RATING_PEACE);
    lang_text_draw_centered(53, 3, 320, 294, 110, FONT_NORMAL_BLACK);
    text_draw_number_centered(peace, 320, 309, 100, FONT_LARGE_BLACK);
    width = text_draw_number(has_peace_goal ? scenario_criteria_peace() : 0,
        '@', " ", 325, 334, FONT_NORMAL_BLACK, 0);
    lang_text_draw(53, 5, 325 + width, 334, FONT_NORMAL_BLACK);
    has_reached = !has_peace_goal || peace >= scenario_criteria_peace();
    draw_rating_column(350, 274, peace, has_reached);

    // favor
    int favor = city_rating_favor();
    int has_favor_goal = !open_play && scenario_criteria_favor_enabled();
    button_border_draw(440, 286, 110, 66, focus_button_id == SELECTED_RATING_FAVOR);
    lang_text_draw_centered(53, 4, 440, 294, 110, FONT_NORMAL_BLACK);
    text_draw_number_centered(favor, 440, 309, 100, FONT_LARGE_BLACK);
    width = text_draw_number(has_favor_goal ? scenario_criteria_favor() : 0,
        '@', " ", 445, 334, FONT_NORMAL_BLACK, 0);
    lang_text_draw(53, 5, 445 + width, 334, FONT_NORMAL_BLACK);
    has_reached = !has_favor_goal || favor >= scenario_criteria_favor();
    draw_rating_column(470, 274, favor, has_reached);

    // bottom info box
    inner_panel_draw(64, 356, 32, 4);
    switch (city_rating_selected()) {
        case SELECTED_RATING_CULTURE:
            lang_text_draw(53, 1, 72, 359, FONT_NORMAL_WHITE);
            if (culture <= 90) {
                lang_text_draw_multiline(53, 9 + city_rating_explanation_for(SELECTED_RATING_CULTURE),
                    72, 374, 496, FONT_NORMAL_WHITE);
            } else {
                lang_text_draw_multiline(53, 50, 72, 374, 496, FONT_NORMAL_WHITE);
            }
            break;
        case SELECTED_RATING_PROSPERITY:
        {
            int line_width;
            lang_text_draw(53, 2, 72, 359, FONT_NORMAL_WHITE);
            if (prosperity <= 90) {
                line_width = lang_text_draw_multiline(53, 16 + city_rating_explanation_for(SELECTED_RATING_PROSPERITY),
                    72, 374, 496, FONT_NORMAL_WHITE);
            } else {
                line_width = lang_text_draw_multiline(53, 51, 72, 374, 496, FONT_NORMAL_WHITE);
            }
            if (config_get(CONFIG_UI_SHOW_MAX_PROSPERITY)) {
                int max = calc_bound(city_ratings_prosperity_max(), 0, 100);
                if (prosperity < max) {
                    int width = lang_text_draw(CUSTOM_TRANSLATION, TR_ADVISOR_MAX_ATTAINABLE_PROSPERITY_IS, 72, 374 + line_width, FONT_NORMAL_WHITE);
                    text_draw_number(max, 0, ".", 72 + width, 374 + line_width, FONT_NORMAL_WHITE, 0);
                }
            }
            break;
        }
        case SELECTED_RATING_PEACE:
            lang_text_draw(53, 3, 72, 359, FONT_NORMAL_WHITE);
            if (peace <= 90) {
                lang_text_draw_multiline(53, 41 + city_rating_explanation_for(SELECTED_RATING_PEACE),
                    72, 374, 496, FONT_NORMAL_WHITE);
            } else {
                lang_text_draw_multiline(53, 52, 72, 374, 496, FONT_NORMAL_WHITE);
            }
            break;
        case SELECTED_RATING_FAVOR:
            lang_text_draw(53, 4, 72, 359, FONT_NORMAL_WHITE);
            if (favor <= 90) {
                lang_text_draw_multiline(53, 27 + city_rating_explanation_for(SELECTED_RATING_FAVOR),
                    72, 374, 496, FONT_NORMAL_WHITE);
            } else {
                lang_text_draw_multiline(53, 53, 72, 374, 496, FONT_NORMAL_WHITE);
            }
            break;
        default:
            lang_text_draw_centered(53, 8, 72, 380, 496, FONT_NORMAL_WHITE);
            break;
    }

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    button_border_draw(80, 286, 110, 66, focus_button_id == SELECTED_RATING_CULTURE);
    button_border_draw(200, 286, 110, 66, focus_button_id == SELECTED_RATING_PROSPERITY);
    button_border_draw(320, 286, 110, 66, focus_button_id == SELECTED_RATING_PEACE);
    button_border_draw(440, 286, 110, 66, focus_button_id == SELECTED_RATING_FAVOR);
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, rating_buttons, 4, &focus_button_id);
}

static void button_rating(int rating, int param2)
{
    city_rating_select(rating);
    window_invalidate();
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    switch (focus_button_id) {
        case SELECTED_RATING_CULTURE:
            r->text_id = 102;
            break;
        case SELECTED_RATING_PROSPERITY:
            r->text_id = 103;
            break;
        case SELECTED_RATING_PEACE:
            r->text_id = 104;
            break;
        case SELECTED_RATING_FAVOR:
            r->text_id = 105;
            break;
    }
}

const advisor_window_type *window_advisor_ratings(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
