#include "Advisors_private.h"

#include "Tooltip.h"
#include "Window.h"

#include "city/finance.h"
#include "city/labor.h"
#include "core/calc.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void arrowButtonWages(int isDown, int param2);
static void buttonPriority(int category, int param2);
static void buttonSetPriority(int newPriority, int param2);

static generic_button categoryButtons[] = {
	{40, 77, 600, 99, GB_IMMEDIATE, buttonPriority, button_none, 0, 0},
	{40, 102, 600, 124, GB_IMMEDIATE, buttonPriority, button_none, 1, 0},
	{40, 127, 600, 149, GB_IMMEDIATE, buttonPriority, button_none, 2, 0},
	{40, 152, 600, 174, GB_IMMEDIATE, buttonPriority, button_none, 3, 0},
	{40, 177, 600, 199, GB_IMMEDIATE, buttonPriority, button_none, 4, 0},
	{40, 202, 600, 224, GB_IMMEDIATE, buttonPriority, button_none, 5, 0},
	{40, 227, 600, 249, GB_IMMEDIATE, buttonPriority, button_none, 6, 0},
	{40, 252, 600, 274, GB_IMMEDIATE, buttonPriority, button_none, 7, 0},
	{40, 277, 600, 299, GB_IMMEDIATE, buttonPriority, button_none, 8, 0},
};

static arrow_button wageButtons[] = {
	{158, 354, 17, 24, arrowButtonWages, 1, 0},
	{182, 354, 15, 24, arrowButtonWages, 0, 0}
};

static int focusButtonId;
static int arrowButtonFocus;

// labor priority stuff
static int prioritySelectedCategory;
static int priorityMaxItems;
static int priorityFocusButtonId;

static generic_button priorityButtons[] = {
	{180, 256, 460, 281, GB_IMMEDIATE, buttonSetPriority, button_none, 0, 0}, // no prio
	{178, 221, 205, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 1, 0},
	{210, 221, 237, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 2, 0},
	{242, 221, 269, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 3, 0},
	{274, 221, 301, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 4, 0},
	{306, 221, 333, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 5, 0},
	{338, 221, 365, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 6, 0},
	{370, 221, 397, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 7, 0},
	{402, 221, 429, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 8, 0},
	{434, 221, 461, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 9, 0},
};

void UI_Advisor_Labor_drawBackground(int *advisorHeight)
{
    graphics_in_dialog();

	*advisorHeight = 26;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS), 10, 10);
	
	Widget_GameText_draw(50, 0, 60, 12, FONT_LARGE_BLACK);
	
	// table headers
	Widget_GameText_draw(50, 21, 60, 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 22, 170, 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 23, 400, 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 24, 500, 56, FONT_SMALL_PLAIN);

	// xx employed, yy unemployed
	int width = Widget_Text_drawNumber(Data_CityInfo.workersEmployed, '@', " ", 32, 320, FONT_NORMAL_BLACK);
	width += Widget_GameText_draw(50, 12, 32 + width, 320, FONT_NORMAL_BLACK);
	width += Widget_Text_drawNumber(Data_CityInfo.workersUnemployed, '@', " ", 50 + width, 320, FONT_NORMAL_BLACK);
	width += Widget_GameText_draw(50, 13, 50 + width, 320, FONT_NORMAL_BLACK);
	width += Widget_Text_drawNumber(Data_CityInfo.unemploymentPercentage, '@', "%)", 50 + width, 320, FONT_NORMAL_BLACK);

	// wages panel
	inner_panel_draw(64, 350, 32, 2);
	Widget_GameText_draw(50, 14, 70, 359, FONT_NORMAL_WHITE);
	width = Widget_Text_drawNumber(Data_CityInfo.wages, '@', " ", 230, 359, FONT_NORMAL_WHITE);
	width += Widget_GameText_draw(50, 15, 230 + width, 359, FONT_NORMAL_WHITE);
	width += Widget_GameText_draw(50, 18, 230 + width, 359, FONT_NORMAL_WHITE);
	width += Widget_Text_drawNumber(Data_CityInfo.wagesRome, '@', " )", 230 + width, 359, FONT_NORMAL_WHITE);

	// estimated wages
	width = Widget_GameText_draw(50, 19, 64, 390, FONT_NORMAL_BLACK);
	width += Widget_Text_drawMoney(Data_CityInfo.estimatedYearlyWages, 64 + width, 390, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

void UI_Advisor_Labor_drawForeground()
{
    graphics_in_dialog();

    arrow_buttons_draw(0, 0, wageButtons, 2);

	inner_panel_draw(32, 70, 36, 15);

	for (int i = 0; i < 9; i++) {
		int focus = i == focusButtonId - 1;
        int y_offset = 82 + 25 * i;
		button_border_draw(40, 77 + 25 * i, 560, 22, focus);
		if (Data_CityInfo.laborCategory[i].priority) {
			Graphics_drawImage(image_group(GROUP_LABOR_PRIORITY_LOCK), 70, y_offset - 2);
			Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].priority, '@', " ",
				90, y_offset, FONT_NORMAL_WHITE);
		}
		Widget_GameText_draw(50, i + 1, 170, y_offset, FONT_NORMAL_WHITE);
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersNeeded, '@', " ",
			410, y_offset, FONT_NORMAL_WHITE);
		font_t font = FONT_NORMAL_WHITE;
		if (Data_CityInfo.laborCategory[i].workersNeeded != Data_CityInfo.laborCategory[i].workersAllocated) {
			font = FONT_NORMAL_RED;
		}
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersAllocated, '@', " ",
			510, y_offset, font);
	}
	graphics_reset_dialog();
}

void UI_Advisor_Labor_handleMouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
	if (!generic_buttons_handle_mouse(m_dialog, 0, 0, categoryButtons, 9, &focusButtonId)) {
		arrowButtonFocus = arrow_buttons_handle_mouse(m_dialog, 0, 0, wageButtons, 2);
	}
}

static void arrowButtonWages(int isDown, int param2)
{
	if (isDown == 1) {
		Data_CityInfo.wages--;
	} else if (isDown == 0) {
		Data_CityInfo.wages++;
	}
	Data_CityInfo.wages = calc_bound(Data_CityInfo.wages, 0, 100);
	city_finance_estimate_wages();
	city_finance_calculate_totals();
	UI_Window_requestRefresh();
}

static void buttonPriority(int category, int param2)
{
	prioritySelectedCategory = category;
	priorityMaxItems = city_labor_max_selectable_priority(category);
	UI_Window_goTo(Window_LaborPriorityDialog);
}

int UI_Advisor_Labor_getTooltip()
{
	if (focusButtonId) {
		return 90;
	} else if (arrowButtonFocus) {
		return 91;
	} else {
		return 0;
	}
}

void UI_LaborPriorityDialog_drawBackground()
{
	int dummy;

	UI_Advisor_Labor_drawBackground(&dummy);
	UI_Advisor_Labor_drawForeground();

    graphics_in_dialog();
	outer_panel_draw(160, 176, 20, 9);
	Widget_GameText_drawCentered(50, 25, 160, 185, 320, FONT_LARGE_BLACK);
	for (int i = 0; i < 9; i++) {
		Graphics_drawRect(178 + 32 * i, 221, 27, 27, COLOR_BLACK);
		Widget_GameText_drawCentered(50, 27 + i, 178 + 32 * i, 224, 27, FONT_LARGE_BLACK);
		if (i >= priorityMaxItems) {
			Graphics_shadeRect(179 + 32 * i, 222, 25, 25, 1);
		}
	}

	Graphics_drawRect(180, 256, 280, 25, COLOR_BLACK);
	Widget_GameText_drawCentered(50, 26, 180, 263, 280, FONT_NORMAL_BLACK);
	Widget_GameText_drawCentered(13, 3, 160, 296, 320, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

void UI_LaborPriorityDialog_drawForeground()
{
    graphics_in_dialog();
	
	color_t color;
	for (int i = 0; i < 9; i++) {
		color = COLOR_BLACK;
		if (i == priorityFocusButtonId - 2) {
			color = COLOR_RED;
		}
		Graphics_drawRect(178 + 32 * i, 221, 27, 27, color);
	}
	color = COLOR_BLACK;
	if (priorityFocusButtonId == 1) {
		color = COLOR_RED;
	}
	Graphics_drawRect(180, 256, 280, 25, color);

    graphics_reset_dialog();
}

void UI_LaborPriorityDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, priorityButtons, 1 + priorityMaxItems, &priorityFocusButtonId);
	}
}

static void buttonSetPriority(int newPriority, int param2)
{
    city_labor_set_priority(prioritySelectedCategory, newPriority);
	UI_Window_goTo(Window_Advisors);
}

void UI_LaborPriorityDialog_getTooltip(struct TooltipContext *c)
{
	if (!priorityFocusButtonId) {
		return;
	}
	c->type = TooltipType_Button;
	if (priorityFocusButtonId == 1) {
		c->textId = 92;
	} else {
		c->textId = 93;
	}
}
