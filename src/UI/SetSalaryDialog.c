#include "AllWindows.h"
#include "Advisors_private.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/ratings.h"
#include "graphics/generic_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void buttonCancel(int param1, int param2);
static void buttonSetSalary(int rank, int param2);

static generic_button buttons[] = {
	{240, 395, 400, 415, GB_IMMEDIATE, buttonCancel, button_none, 0, 0},
	{144, 85, 432, 105, GB_IMMEDIATE, buttonSetSalary, button_none, 0, 0},
	{144, 105, 432, 125, GB_IMMEDIATE, buttonSetSalary, button_none, 1, 0},
	{144, 125, 432, 145, GB_IMMEDIATE, buttonSetSalary, button_none, 2, 0},
	{144, 145, 432, 165, GB_IMMEDIATE, buttonSetSalary, button_none, 3, 0},
	{144, 165, 432, 185, GB_IMMEDIATE, buttonSetSalary, button_none, 4, 0},
	{144, 185, 432, 205, GB_IMMEDIATE, buttonSetSalary, button_none, 5, 0},
	{144, 205, 432, 225, GB_IMMEDIATE, buttonSetSalary, button_none, 6, 0},
	{144, 225, 432, 245, GB_IMMEDIATE, buttonSetSalary, button_none, 7, 0},
	{144, 245, 432, 265, GB_IMMEDIATE, buttonSetSalary, button_none, 8, 0},
	{144, 265, 432, 285, GB_IMMEDIATE, buttonSetSalary, button_none, 9, 0},
	{144, 285, 432, 305, GB_IMMEDIATE, buttonSetSalary, button_none, 10, 0},
};

static int focusButtonId;

void UI_SetSalaryDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();
	UI_SetSalaryDialog_drawForeground();
}

void UI_SetSalaryDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	outer_panel_draw(baseOffsetX + 128, baseOffsetY + 32, 24, 25);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + 16,
		baseOffsetX + 144, baseOffsetY + 48);
	lang_text_draw_centered(52, 15,
		baseOffsetX + 144, baseOffsetY + 48, 368, FONT_LARGE_BLACK);

	inner_panel_draw(baseOffsetX + 144, baseOffsetY + 80, 22, 15);

	for (int rank = 0; rank < 11; rank++) {
		font_t font = (focusButtonId == rank + 2) ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
		int width = lang_text_draw(52, rank + 4,
			baseOffsetX + 176, baseOffsetY + 90 + 20 * rank, font);
		text_draw_money(city_emperor_salary_for_rank(rank),
			baseOffsetX + 176 + width, baseOffsetY + 90 + 20 * rank, font);
	}

	if (Data_CityInfo.victoryContinueMonths <= 0) {
		if (Data_CityInfo.salaryRank <= Data_CityInfo.playerRank) {
			lang_text_draw_multiline(52, 76,
				baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
		} else {
			lang_text_draw_multiline(52, 71,
				baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
		}
	} else {
		lang_text_draw_multiline(52, 77,
				baseOffsetX + 152, baseOffsetY + 336, 336, FONT_NORMAL_BLACK);
	}
	button_border_draw(baseOffsetX + 240, baseOffsetY + 395,
		160, 20, focusButtonId == 1);
	lang_text_draw_centered(13, 4, baseOffsetX + 176, baseOffsetY + 400, 288, FONT_NORMAL_BLACK);
}

void UI_SetSalaryDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0,
			buttons, 12, &focusButtonId);
	}
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void buttonSetSalary(int rank, int param2)
{
	if (Data_CityInfo.victoryContinueMonths <= 0) {
		city_emperor_set_salary_rank(rank);
		city_finance_update_salary();
		city_ratings_update_favor_explanation();
		UI_Window_goTo(Window_Advisors);
	}
}

