#include "AllWindows.h"
#include "MessageDialog.h"
#include "../Graphics.h"
#include "Advisors_private.h"

#include "building/warehouse.h"
#include "city/constants.h"
#include "city/finance.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"

static void drawButtons();
static void buttonGod(int god, int param2);
static void buttonSize(int size, int param2);
static void buttonHelp(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonHoldFestival(int param1, int param2);

static image_button imageButtonsBottom[] = {
	{58, 316, 27, 27, IB_NORMAL, 134, 0, buttonHelp, button_none, 0, 0, 1},
	{558, 319, 24, 24, IB_NORMAL, 134, 4, buttonClose, button_none, 0, 0, 1},
	{358, 317, 34, 34, IB_NORMAL, 96, 0, buttonHoldFestival, button_none, 1, 0, 1},
	{400, 317, 34, 34, IB_NORMAL, 96, 4, buttonClose, button_none, 0, 0, 1},
};

static generic_button buttonsGodsSize[] = {
	{70, 96, 150, 186, GB_IMMEDIATE, buttonGod, button_none, 0, 0},
	{170, 96, 250, 186, GB_IMMEDIATE, buttonGod, button_none, 1, 0},
	{270, 96, 350, 186, GB_IMMEDIATE, buttonGod, button_none, 2, 0},
	{370, 96, 450, 186, GB_IMMEDIATE, buttonGod, button_none, 3, 0},
	{470, 96, 550, 186, GB_IMMEDIATE, buttonGod, button_none, 4, 0},
	{102, 216, 532, 242, GB_IMMEDIATE, buttonSize, button_none, 1, 0},
	{102, 246, 532, 272, GB_IMMEDIATE, buttonSize, button_none, 2, 0},
	{102, 276, 532, 302, GB_IMMEDIATE, buttonSize, button_none, 3, 0},
};

static int focusButtonId;
static int focusImageButtonId;

void UI_HoldFestivalDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();

    graphics_in_dialog();

	outer_panel_draw(48, 48, 34, 20);
	lang_text_draw_centered(58, 25 + Data_CityInfo.festivalGod, 48, 60, 544, FONT_LARGE_BLACK);
	for (int god = 0; god < 5; god++) {
		if (god == Data_CityInfo.festivalGod) {
			button_border_draw(100 * god + 66, 92, 90, 100, 1);
			Graphics_drawImage(image_group(GROUP_PANEL_WINDOWS) + god + 21, 100 * god + 70, 96);
		} else {
			Graphics_drawImage(image_group(GROUP_PANEL_WINDOWS) + god + 16, 100 * god + 70, 96);
		}
	}
	drawButtons();
	lang_text_draw(58, 30 + Data_CityInfo.festivalSize, 180, 322, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

void UI_HoldFestivalDialog_drawForeground()
{
    graphics_in_dialog();
    drawButtons();
    image_buttons_draw(0, 0, imageButtonsBottom, 4);
    graphics_reset_dialog();
}

static void drawButtons()
{
	int width;

	// small festival
	button_border_draw(102, 216, 430, 26, focusButtonId == 6);
	width = lang_text_draw(58, 31, 110, 224, FONT_NORMAL_BLACK);
	lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostSmall, 110 + width, 224, FONT_NORMAL_BLACK);

	// large festival
	button_border_draw(102, 246, 430, 26, focusButtonId == 7);
	width = lang_text_draw(58, 32, 110, 254, FONT_NORMAL_BLACK);
	lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostLarge, 110 + width, 254, FONT_NORMAL_BLACK);

	// grand festival
	button_border_draw(102, 276, 430, 26, focusButtonId == 8);
	width = lang_text_draw(58, 33, 110, 284, FONT_NORMAL_BLACK);
	width += lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostGrand,
		110 + width, 284, FONT_NORMAL_BLACK);
	width += lang_text_draw_amount(8, 10, Data_CityInfo.festivalWineGrand,
		120 + width, 284, FONT_NORMAL_BLACK);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WINE,
		120 + width, 279);
	
	// greying out of buttons
	if (city_finance_out_of_money()) {
		Graphics_shadeRect(104, 218, 426, 22, 0);
		Graphics_shadeRect(104, 248, 426, 22, 0);
		Graphics_shadeRect(104, 278, 426, 22, 0);
	} else if (Data_CityInfo.festivalNotEnoughWine) {
		Graphics_shadeRect(104, 278, 426, 22, 0);
	}
}

void UI_HoldFestivalDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
		return;
	}

	const mouse *m_dialog = mouse_in_dialog(m);
	image_buttons_handle_mouse(m_dialog, 0, 0, imageButtonsBottom, 4, &focusImageButtonId);
	generic_buttons_handle_mouse(m_dialog, 0, 0, buttonsGodsSize, 8, &focusButtonId);
	if (focusImageButtonId) {
		focusButtonId = 0;
	}
}

static void buttonGod(int god, int param2)
{
	Data_CityInfo.festivalGod = god;
	UI_Window_requestRefresh();
}

static void buttonSize(int size, int param2)
{
	if (!city_finance_out_of_money()) {
		if (size != FESTIVAL_GRAND || !Data_CityInfo.festivalNotEnoughWine) {
			Data_CityInfo.festivalSize = size;
			UI_Window_requestRefresh();
		}
	}
}

static void buttonHelp(int param1, int param2)
{
	UI_MessageDialog_show(MessageDialog_EntertainmentAdvisor, 1);
}

static void buttonClose(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void buttonHoldFestival(int param1, int param2)
{
	if (city_finance_out_of_money()) {
		return;
	}
	Data_CityInfo.plannedFestivalGod = Data_CityInfo.festivalGod;
	Data_CityInfo.plannedFestivalSize = Data_CityInfo.festivalSize;
	int cost;
	if (Data_CityInfo.festivalSize == FESTIVAL_SMALL) {
		Data_CityInfo.plannedFestivalMonthsToGo = 2;
		cost = Data_CityInfo.festivalCostSmall;
	} else if (Data_CityInfo.festivalSize == FESTIVAL_LARGE) {
		Data_CityInfo.plannedFestivalMonthsToGo = 3;
		cost = Data_CityInfo.festivalCostLarge;
	} else {
		Data_CityInfo.plannedFestivalMonthsToGo = 4;
		cost = Data_CityInfo.festivalCostGrand;
	}

	city_finance_process_sundry(cost);

	if (Data_CityInfo.festivalSize == FESTIVAL_GRAND) {
		building_warehouses_remove_resource(RESOURCE_WINE, Data_CityInfo.festivalWineGrand);
	}
	UI_Window_goTo(Window_Advisors);
}

void UI_HoldFestivalDialog_getTooltip(struct TooltipContext *c)
{
	if (!focusImageButtonId && (!focusButtonId || focusButtonId > 5)) {
		return;
	}
	c->type = TooltipType_Button;
	// image buttons
	switch (focusImageButtonId) {
		case 1: c->textId = 1; break;
		case 2: c->textId = 2; break;
		case 3: c->textId = 113; break;
		case 4: c->textId = 114; break;
	}
	// gods
	switch (focusButtonId) {
		case 1: c->textId = 115; break;
		case 2: c->textId = 116; break;
		case 3: c->textId = 117; break;
		case 4: c->textId = 118; break;
		case 5: c->textId = 119; break;
	}
}

