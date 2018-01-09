#include "Advisors_private.h"
#include "Window.h"

#include "city/finance.h"
#include "core/calc.h"
#include "graphics/arrow_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void buttonChangeTaxes(int isDown, int param2);

static arrow_button arrowButtonsTaxes[] = {
	{180, 75, 17, 24, buttonChangeTaxes, 1, 0},
	{204, 75, 15, 24, buttonChangeTaxes, 0, 0}
};

static int arrowButtonFocus;

static void draw_row(int group, int number, int y, int value_last_year, int value_this_year)
{
    lang_text_draw(group, number, 80, y, FONT_NORMAL_BLACK);
    text_draw_number(value_last_year, '@', " ", 290, y, FONT_NORMAL_BLACK);
    text_draw_number(value_this_year, '@', " ", 430, y, FONT_NORMAL_BLACK);
}

void UI_Advisor_Financial_drawBackground(int *advisorHeight)
{
    graphics_in_dialog();

	*advisorHeight = 26;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	image_draw(image_group(GROUP_ADVISOR_ICONS) + 10, 10, 10);

	lang_text_draw(60, 0, 60, 12, FONT_LARGE_BLACK);
	inner_panel_draw(64, 48, 34, 5);

	int width;
    int treasury = city_finance_treasury();
	if (treasury < 0) {
		width = lang_text_draw(60, 3, 70, 58, FONT_NORMAL_RED);
		lang_text_draw_amount(8, 0,
			-treasury, 72 + width, 58, FONT_NORMAL_RED
		);
	} else {
		width = lang_text_draw(60, 2, 70, 58, FONT_NORMAL_WHITE);
		lang_text_draw_amount(8, 0,
			treasury, 72 + width, 58, FONT_NORMAL_WHITE
		);
	}

	// tax percentage and estimated income
	lang_text_draw(60, 1, 70, 81, FONT_NORMAL_WHITE);
	width = text_draw_percentage(Data_CityInfo.taxPercentage,
		240, 81, FONT_NORMAL_WHITE
	);
	width += lang_text_draw(60, 4, 240 + width, 81, FONT_NORMAL_WHITE);
	lang_text_draw_amount(8, 0,
		Data_CityInfo.estimatedTaxIncome, 240 + width, 81, FONT_NORMAL_WHITE
	);

	// percentage taxpayers
	width = text_draw_percentage(Data_CityInfo.percentageTaxedPeople,
		70, 103, FONT_NORMAL_WHITE
	);
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

    graphics_reset_dialog();
}

void UI_Advisor_Financial_drawForeground()
{
    graphics_in_dialog();
    arrow_buttons_draw(0, 0, arrowButtonsTaxes, 2);
    graphics_reset_dialog();
}

void UI_Advisor_Financial_handleMouse(const mouse *m)
{
	arrowButtonFocus = arrow_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, arrowButtonsTaxes, 2);
}

static void buttonChangeTaxes(int isDown, int param2)
{
	if (isDown) {
		--Data_CityInfo.taxPercentage;
	} else {
		++Data_CityInfo.taxPercentage;
	}
	Data_CityInfo.taxPercentage = calc_bound(Data_CityInfo.taxPercentage, 0, 25);

	city_finance_estimate_taxes();
	city_finance_calculate_totals();
	UI_Window_requestRefresh();
}

int UI_Advisor_Financial_getTooltip()
{
	if (arrowButtonFocus) {
		return 120;
	} else {
		return 0;
	}
}
