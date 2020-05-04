#include "financial.h"

#include "city/finance.h"
#include "core/calc.h"
#include "graphics/arrow_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"

#define ADVISOR_HEIGHT 26

static void button_change_taxes(int is_down, int param2);

static arrow_button arrow_buttons_taxes[] = {
    {180, 75, 17, 24, button_change_taxes, 1, 0},
    {204, 75, 15, 24, button_change_taxes, 0, 0}
};

static int arrow_button_focus;

static void draw_row(int group, int number, int y, int value_last_year, int value_this_year)
{
    lang_text_draw(group, number, 80, y, FONT_NORMAL_BLACK);
    text_draw_number(value_last_year, '@', " ", 290, y, FONT_NORMAL_BLACK);
    text_draw_number(value_this_year, '@', " ", 430, y, FONT_NORMAL_BLACK);
}

static int draw_background(void)
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 10, 10, 10);

    lang_text_draw(60, 0, 60, 12, FONT_LARGE_BLACK);
    inner_panel_draw(64, 48, 34, 5);

    int treasury = city_finance_treasury();
    const finance_overview *last_year = city_finance_overview_last_year();
    const finance_overview *this_year = city_finance_overview_this_year();

    int width;
    if (treasury < 0) {
        width = lang_text_draw(60, 3, 70, 58, FONT_NORMAL_RED);
        lang_text_draw_amount(8, 0, -treasury, 72 + width, 58, FONT_NORMAL_RED);
    } else {
        width = lang_text_draw(60, 2, 70, 58, FONT_NORMAL_WHITE);
        lang_text_draw_amount(8, 0, treasury, 72 + width, 58, FONT_NORMAL_WHITE);
    }

    // tax percentage and estimated income
    lang_text_draw(60, 1, 70, 81, FONT_NORMAL_WHITE);
    width = text_draw_percentage(city_finance_tax_percentage(), 240, 81, FONT_NORMAL_WHITE);
    width += lang_text_draw(60, 4, 240 + width, 81, FONT_NORMAL_WHITE);
    lang_text_draw_amount(8, 0, city_finance_estimated_tax_income(), 240 + width, 81, FONT_NORMAL_WHITE);

    // percentage taxpayers
    width = text_draw_percentage(city_finance_percentage_taxed_people(), 70, 103, FONT_NORMAL_WHITE);
    lang_text_draw(60, 5, 70 + width, 103, FONT_NORMAL_WHITE);

    // table headers
    lang_text_draw(60, 6, 270, 133, FONT_NORMAL_BLACK);
    lang_text_draw(60, 7, 400, 133, FONT_NORMAL_BLACK);

    // income
    draw_row(60, 8, 155, last_year->income.taxes, this_year->income.taxes);
    draw_row(60, 9, 170, last_year->income.exports, this_year->income.exports);
    draw_row(60, 20, 185, last_year->income.donated, this_year->income.donated);

    graphics_draw_horizontal_line(280, 350, 198, COLOR_BLACK);
    graphics_draw_horizontal_line(420, 490, 198, COLOR_BLACK);

    draw_row(60, 10, 203, last_year->income.total, this_year->income.total);

    // expenses
    draw_row(60, 11, 227, last_year->expenses.imports, this_year->expenses.imports);
    draw_row(60, 12, 242, last_year->expenses.wages, this_year->expenses.wages);
    draw_row(60, 13, 257, last_year->expenses.construction, this_year->expenses.construction);

    // interest (with percentage)
    width = lang_text_draw(60, 14, 80, 272, FONT_NORMAL_BLACK);
    text_draw_percentage(10, 80 + width, 272, FONT_NORMAL_BLACK);
    text_draw_number(last_year->expenses.interest, '@', " ", 290, 272, FONT_NORMAL_BLACK);
    text_draw_number(last_year->expenses.interest, '@', " ", 430, 272, FONT_NORMAL_BLACK);

    draw_row(60, 15, 287, last_year->expenses.salary, this_year->expenses.salary);
    draw_row(60, 16, 302, last_year->expenses.sundries, this_year->expenses.sundries);
    draw_row(60, 21, 317, last_year->expenses.tribute, this_year->expenses.tribute);

    graphics_draw_horizontal_line(280, 350, 330, COLOR_BLACK);
    graphics_draw_horizontal_line(420, 490, 330, COLOR_BLACK);

    draw_row(60, 17, 335, last_year->expenses.total, this_year->expenses.total);
    draw_row(60, 18, 358, last_year->net_in_out, this_year->net_in_out);
    draw_row(60, 19, 381, last_year->balance, this_year->balance);

    return ADVISOR_HEIGHT;
}

static void draw_foreground(void)
{
    arrow_buttons_draw(0, 0, arrow_buttons_taxes, 2);
}

static int handle_mouse(const mouse *m)
{
    return arrow_buttons_handle_mouse(m, 0, 0, arrow_buttons_taxes, 2, &arrow_button_focus);
}

static void button_change_taxes(int is_down, int param2)
{
    city_finance_change_tax_percentage(is_down ? -1 : 1);
    city_finance_estimate_taxes();
    city_finance_calculate_totals();
    window_invalidate();
}

static int get_tooltip_text(void)
{
    if (arrow_button_focus) {
        return 120;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_financial(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
