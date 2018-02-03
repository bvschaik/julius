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

#include "Data/CityInfo.h"

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

static int draw_background()
{
    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 10, 10, 10);

    lang_text_draw(60, 0, 60, 12, FONT_LARGE_BLACK);
    inner_panel_draw(64, 48, 34, 5);

    int width;
    int treasury = city_finance_treasury();
    if (treasury < 0) {
        width = lang_text_draw(60, 3, 70, 58, FONT_NORMAL_RED);
        lang_text_draw_amount(8, 0, -treasury, 72 + width, 58, FONT_NORMAL_RED);
    } else {
        width = lang_text_draw(60, 2, 70, 58, FONT_NORMAL_WHITE);
        lang_text_draw_amount(8, 0, treasury, 72 + width, 58, FONT_NORMAL_WHITE);
    }

    // tax percentage and estimated income
    lang_text_draw(60, 1, 70, 81, FONT_NORMAL_WHITE);
    width = text_draw_percentage(Data_CityInfo.taxPercentage, 240, 81, FONT_NORMAL_WHITE);
    width += lang_text_draw(60, 4, 240 + width, 81, FONT_NORMAL_WHITE);
    lang_text_draw_amount(8, 0, Data_CityInfo.estimatedTaxIncome, 240 + width, 81, FONT_NORMAL_WHITE);

    // percentage taxpayers
    width = text_draw_percentage(Data_CityInfo.percentageTaxedPeople, 70, 103, FONT_NORMAL_WHITE);
    lang_text_draw(60, 5, 70 + width, 103, FONT_NORMAL_WHITE);

    // table headers
    lang_text_draw(60, 6, 270, 133, FONT_NORMAL_BLACK);
    lang_text_draw(60, 7, 400, 133, FONT_NORMAL_BLACK);

    // income
    draw_row(60, 8, 155, Data_CityInfo.financeTaxesLastYear, Data_CityInfo.financeTaxesThisYear);
    draw_row(60, 9, 170, Data_CityInfo.financeExportsLastYear, Data_CityInfo.financeExportsThisYear);
    draw_row(60, 20, 185, Data_CityInfo.financeDonatedLastYear, Data_CityInfo.financeDonatedThisYear);

    graphics_draw_line(280, 198, 350, 198, COLOR_BLACK);
    graphics_draw_line(420, 198, 490, 198, COLOR_BLACK);
    
    draw_row(60, 10, 203, Data_CityInfo.financeTotalIncomeLastYear, Data_CityInfo.financeTotalIncomeThisYear);

    // expenses
    draw_row(60, 11, 227, Data_CityInfo.financeImportsLastYear, Data_CityInfo.financeImportsThisYear);
    draw_row(60, 12, 242, Data_CityInfo.financeWagesLastYear, Data_CityInfo.financeWagesThisYear);
    draw_row(60, 13, 257, Data_CityInfo.financeConstructionLastYear, Data_CityInfo.financeConstructionThisYear);

    // interest (with percentage)
    width = lang_text_draw(60, 14, 80, 272, FONT_NORMAL_BLACK);
    text_draw_percentage(10, 80 + width, 272, FONT_NORMAL_BLACK);
    text_draw_number(Data_CityInfo.financeInterestLastYear, '@', " ", 290, 272, FONT_NORMAL_BLACK);
    text_draw_number(Data_CityInfo.financeInterestThisYear, '@', " ", 430, 272, FONT_NORMAL_BLACK);

    draw_row(60, 15, 287, Data_CityInfo.financeSalaryLastYear, Data_CityInfo.financeSalaryThisYear);
    draw_row(60, 16, 302, Data_CityInfo.financeSundriesLastYear, Data_CityInfo.financeSundriesThisYear);
    draw_row(60, 21, 317, Data_CityInfo.financeTributeLastYear, Data_CityInfo.financeTributeThisYear);

    graphics_draw_line(280, 330, 350, 330, COLOR_BLACK);
    graphics_draw_line(420, 330, 490, 330, COLOR_BLACK);
    
    draw_row(60, 17, 335, Data_CityInfo.financeTotalExpensesLastYear, Data_CityInfo.financeTotalExpensesThisYear);
    draw_row(60, 18, 358, Data_CityInfo.financeNetInOutLastYear, Data_CityInfo.financeNetInOutThisYear);
    draw_row(60, 19, 381, Data_CityInfo.financeBalanceLastYear, Data_CityInfo.financeBalanceThisYear);

    return ADVISOR_HEIGHT;
}

static void draw_foreground()
{
    arrow_buttons_draw(0, 0, arrow_buttons_taxes, 2);
}

static void handle_mouse(const mouse *m)
{
    arrow_button_focus = arrow_buttons_handle_mouse(m, 0, 0, arrow_buttons_taxes, 2);
}

static void button_change_taxes(int is_down, int param2)
{
    if (is_down) {
        --Data_CityInfo.taxPercentage;
    } else {
        ++Data_CityInfo.taxPercentage;
    }
    Data_CityInfo.taxPercentage = calc_bound(Data_CityInfo.taxPercentage, 0, 25);

    city_finance_estimate_taxes();
    city_finance_calculate_totals();
    window_invalidate();
}

static int get_tooltip_text()
{
    if (arrow_button_focus) {
        return 120;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_financial()
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
