#include "AllWindows.h"
#include "Advisors_private.h"

#include "city/emperor.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"

static void buttonSetGift(int param1, int param2);
static void buttonSendGift(int param1, int param2);
static void buttonCancel(int param1, int param2);

static generic_button buttons[] = {
	{208, 213, 528, 233, GB_IMMEDIATE, buttonSetGift, button_none, 1, 0},
	{208, 233, 528, 253, GB_IMMEDIATE, buttonSetGift, button_none, 2, 0},
	{208, 253, 528, 273, GB_IMMEDIATE, buttonSetGift, button_none, 3, 0},
	{128, 336, 368, 356, GB_IMMEDIATE, buttonSendGift, button_none, 0, 0},
	{400, 336, 560, 356, GB_IMMEDIATE, buttonCancel, button_none, 0, 0},
};

static int focusButtonId;

void UI_SendGiftToCaesarDialog_init()
{
	if (Data_CityInfo.giftSizeSelected == 2 &&
		Data_CityInfo.giftCost_lavish > Data_CityInfo.personalSavings) {
		Data_CityInfo.giftSizeSelected = 1;
	}
	if (Data_CityInfo.giftSizeSelected == 1 &&
		Data_CityInfo.giftCost_generous > Data_CityInfo.personalSavings) {
		Data_CityInfo.giftSizeSelected = 0;
	}
}

void UI_SendGiftToCaesarDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();

    graphics_in_dialog();

	outer_panel_draw(96, 144, 30, 15);
	image_draw(image_group(GROUP_RESOURCE_ICONS) + 16, 128, 160);
	lang_text_draw_centered(52, 69, 128, 160, 432, FONT_LARGE_BLACK);

	int width = lang_text_draw(52, 50, 144, 304, FONT_NORMAL_BLACK);
	lang_text_draw_amount(8, 4, Data_CityInfo.giftMonthsSinceLast,
		144 + width, 304, FONT_NORMAL_BLACK);
	lang_text_draw_centered(13, 4, 400, 341, 160, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

void UI_SendGiftToCaesarDialog_drawForeground()
{
    graphics_in_dialog();

	inner_panel_draw(112, 208, 28, 5); // BUGFIX red/white letters overlapping

	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		lang_text_draw(52, 63, 128, 218, FONT_NORMAL_WHITE);
		font_t font = focusButtonId == 1 ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
		int width = lang_text_draw(52, 51 + Data_CityInfo.giftId_modest, 224, 218, font);
		text_draw_money(Data_CityInfo.giftCost_modest, 224 + width, 218, font);
	} else {
		lang_text_draw_multiline(52, 70, 160, 224, 352, FONT_NORMAL_WHITE);
	}
	if (Data_CityInfo.giftCost_generous <= Data_CityInfo.personalSavings) {
		lang_text_draw(52, 64, 128, 238, FONT_NORMAL_WHITE);
		font_t font = focusButtonId == 2 ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
		int width = lang_text_draw(52, 55 + Data_CityInfo.giftId_generous, 224, 238, font);
		text_draw_money(Data_CityInfo.giftCost_generous, 224 + width, 238, font);
	}
	if (Data_CityInfo.giftCost_lavish <= Data_CityInfo.personalSavings) {
		lang_text_draw(52, 65, 128, 258, FONT_NORMAL_WHITE);
		font_t font = focusButtonId == 3 ? FONT_NORMAL_RED : FONT_NORMAL_WHITE;
		int width = lang_text_draw(52, 59 + Data_CityInfo.giftId_lavish, 224, 258, font);
		text_draw_money(Data_CityInfo.giftCost_lavish, 224 + width, 258, font);
	}
	// can give at least one type
	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		lang_text_draw_centered(52, 66 + Data_CityInfo.giftSizeSelected,
			128, 341, 240, FONT_NORMAL_BLACK);
		button_border_draw(128, 336, 240, 20, focusButtonId == 4);
	}
	button_border_draw(400, 336, 160, 20, focusButtonId == 5);

    graphics_reset_dialog();
}

void UI_SendGiftToCaesarDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 5, &focusButtonId);
	}
}

static void buttonSetGift(int param1, int param2)
{
	if ((param1 == 1 && Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) ||
		(param1 == 2 && Data_CityInfo.giftCost_generous <= Data_CityInfo.personalSavings) ||
		(param1 == 3 && Data_CityInfo.giftCost_lavish <= Data_CityInfo.personalSavings)) {
		Data_CityInfo.giftSizeSelected = param1 - 1;
		window_invalidate();
	}
}

static void buttonSendGift(int param1, int param2)
{
	if (Data_CityInfo.giftCost_modest <= Data_CityInfo.personalSavings) {
		city_emperor_send_gift();
		UI_Window_goTo(Window_Advisors);
	}
}

static void buttonCancel(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

