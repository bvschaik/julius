#include "tourism.h"

#include "building/count.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/resource.h"

#include "game/time.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "translation/translation.h"


#define ADVISOR_HEIGHT 27

static void go_back(int param1, int param2);

static generic_button back_button[] = {
    {545, 260, 60, 51, go_back, button_none, 0, 1}
};

static int focus_button_id;

static int draw_background(void)
{
    int val;
    int width;
    int height;
    
    city_finance_calculate_tourism_rating();
    int tourism_lowest_factor = city_finance_tourism_lowest_factor();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    inner_panel_draw(24, 60, 32, 20);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 8, 10, 10);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 8, 555, 265);

    text_draw(translation_for(TR_ADVISOR_HEADER_TOURISM), 60, 12, FONT_LARGE_BLACK, 0);
    text_draw_centered(translation_for(TR_ADVISOR_BACK_BUTTON_TOURISM), 545, 315, 61, FONT_NORMAL_BLACK, 0);

    height = text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESCRIPTION_TEXT), 40, 70, 480, FONT_NORMAL_WHITE, 0);
    width = text_draw(translation_for(TR_ADVISOR_TOURISM_DESCRIPTION_TEXT_2), 40, 80+height, FONT_NORMAL_WHITE, 0);
    width += text_draw(translation_for(TR_ADVISOR_TOURISM_DESCRIPTION_1 + calc_bound(city_finance_tourism_rating() / 15, 0, 8)), 40 + width, 80 + height, FONT_NORMAL_WHITE, 0);
    width += text_draw_money(city_finance_tourism_income_last_month(), 40 + width, 80 + height, FONT_NORMAL_WHITE);
    text_draw(translation_for(TR_BUILDING_INFO_MONTHLY_LEVY), 40 + width, 80 + height, FONT_NORMAL_WHITE, 0);

    if (city_population() < TOURISM_POP_MINIMUM) {
        height += text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_TOO_SMALL), 40, 140, 480, FONT_NORMAL_WHITE, 0);
    }
    else if (city_population() < TOURISM_POP_PENALTY) {
        height += text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_LOW_POP), 40, 140, 480, FONT_NORMAL_WHITE, 0);

    } 
    
    if (city_population() >= TOURISM_POP_MINIMUM) {
        switch (tourism_lowest_factor) {
        case 1:
            height += text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_LOW_COVERAGE), 40, 140 + height, 480, FONT_NORMAL_WHITE, 0);
            break;
        case 2:
            height += text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_LOW_SENTIMENT), 40, 140 + height, 480, FONT_NORMAL_WHITE, 0);
            break;
        case 3:
            height += text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_LOW_DESIRABILTY), 40, 140 + height, 480, FONT_NORMAL_WHITE, 0);
            break;
        }

        if (building_count_grand_temples_active()) {
            text_draw_multiline(translation_for(TR_ADVISOR_TOURISM_DESC_MONUMENTS_PRESENT), 40, 150 + height, 480, FONT_NORMAL_WHITE, 0);
        }
    }

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    if (focus_button_id == 0) {
        button_border_draw(545, 260, 60, 51, 0);
    } else if (focus_button_id == 1) {
        button_border_draw(545, 260, 60, 51, 1);
    }
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, back_button, 1, &focus_button_id);
}

static void go_back(int param1, int param2)
{
    window_advisors_show_advisor(9);
    window_invalidate();
}

static int get_tooltip_text(void) {
    return 0;
}

const advisor_window_type *window_advisor_tourism(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
