#include "Advisors_private.h"
#include "PopupDialog.h"
#include "Window.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "empire/city.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/property.h"
#include "scenario/request.h"

static void buttonDonateToCity(int param1, int param2);
static void buttonSetSalary(int param1, int param2);
static void buttonGiftToCaesar(int param1, int param2);
static void buttonRequest(int index, int param2);

static int getRequestStatus(int index);

static void confirmNothing(int accepted);
static void confirmSendTroops(int accepted);
static void confirmSendGoods(int accepted);

static generic_button imperialButtons[] = {
	{320, 367, 570, 387, GB_IMMEDIATE, buttonDonateToCity, button_none, 0, 0},
	{70, 393, 570, 413, GB_IMMEDIATE, buttonSetSalary, button_none, 0, 0},
	{320, 341, 570, 361, GB_IMMEDIATE, buttonGiftToCaesar, button_none, 0, 0},
	{38, 96, 598, 136, GB_IMMEDIATE, buttonRequest, button_none, 0, 0},
	{38, 138, 598, 178, GB_IMMEDIATE, buttonRequest, button_none, 1, 0},
	{38, 180, 598, 220, GB_IMMEDIATE, buttonRequest, button_none, 2, 0},
	{38, 222, 598, 262, GB_IMMEDIATE, buttonRequest, button_none, 3, 0},
	{38, 264, 598, 304, GB_IMMEDIATE, buttonRequest, button_none, 4, 0},
};

static int focusButtonId;
static int selectedRequestId;

static void draw_request(int index, const scenario_request *request)
{
    if (index >= 5) {
        return;
    }

    button_border_draw(38, 96 + 42 * index, 560, 40, 0);
    text_draw_number(request->amount, '@', " ", 40, 102 + 42 * index, FONT_NORMAL_WHITE);
    int resourceOffset = request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON);
    Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + resourceOffset, 110, 100 + 42 * index);
    lang_text_draw(23, request->resource, 150, 102 + 42 * index, FONT_NORMAL_WHITE);
    
    int width = lang_text_draw_amount(8, 4, request->months_to_comply, 310, 102 + 42 * index, FONT_NORMAL_WHITE);
    lang_text_draw(12, 2, 310 + width, 102 + 42 * index, FONT_NORMAL_WHITE);

    if (request->resource == RESOURCE_DENARII) {
        // request for money
        int treasury = city_finance_treasury();
        width = text_draw_number(treasury, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE);
        width += lang_text_draw(52, 44, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        if (treasury < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    } else {
        // normal goods request
        int resourceId = request->resource;
        width = text_draw_number(Data_CityInfo.resourceStored[resourceId], '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE);
        width += lang_text_draw(52, 43, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        if (Data_CityInfo.resourceStored[resourceId] < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    }
}

void UI_Advisor_Imperial_drawBackground(int *advisorHeight)
{
	city_emperor_calculate_gift_costs();

    graphics_in_dialog();

	*advisorHeight = 27;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + 2, 10, 10);

	text_draw(scenario_player_name(), 60, 12, FONT_LARGE_BLACK, 0);

	int width = lang_text_draw(52, 0, 60, 44, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.ratingFavor, '@', " ", 60 + width, 44, FONT_NORMAL_BLACK);

	lang_text_draw_multiline(52, Data_CityInfo.ratingFavor / 5 + 22, 60, 60, 544, FONT_NORMAL_BLACK);

	inner_panel_draw(32, 90, 36, 14);
	
	int numRequests = 0;
	if (Data_CityInfo.distantBattleMonthsToBattle > 0 && Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
		// can send to distant battle
		button_border_draw(38, 96, 560, 40, 0);
		Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS, 50, 106);
		width = lang_text_draw(52, 72, 80, 102, FONT_NORMAL_WHITE);
		lang_text_draw(21, empire_city_get(Data_CityInfo.distantBattleCityId)->name_id, 50 + width, 102, FONT_NORMAL_WHITE);
		int strengthTextId;
		if (Data_CityInfo.distantBattleEnemyStrength < 46) {
			strengthTextId = 73;
		} else if (Data_CityInfo.distantBattleEnemyStrength < 89) {
			strengthTextId = 74;
		} else {
			strengthTextId = 75;
		}
		width = lang_text_draw(52, strengthTextId, 80, 120, FONT_NORMAL_WHITE);
		lang_text_draw_amount(8, 4, Data_CityInfo.distantBattleMonthsToBattle, 80 + width, 120, FONT_NORMAL_WHITE);
		numRequests = 1;
	}
	numRequests = scenario_request_foreach_visible(numRequests, draw_request);
	if (!numRequests) {
		lang_text_draw_multiline(52, 21, 64, 160, 512, FONT_NORMAL_WHITE);
	}
	graphics_reset_dialog();
}

void UI_Advisor_Imperial_drawForeground()
{
    graphics_in_dialog();

	inner_panel_draw(64, 324, 32, 6);

	lang_text_draw(32, Data_CityInfo.playerRank, 72, 338, FONT_LARGE_BROWN);
	
	int width = lang_text_draw(52, 1, 72, 372, FONT_NORMAL_WHITE);
	text_draw_money(Data_CityInfo.personalSavings, 80 + width, 372, FONT_NORMAL_WHITE);

	button_border_draw(320, 367, 250, 20, focusButtonId == 1);
	lang_text_draw_centered(52, 2, 320, 372, 250, FONT_NORMAL_WHITE);

	button_border_draw(70, 393, 500, 20, focusButtonId == 2);
	width = lang_text_draw(52, Data_CityInfo.salaryRank + 4, 120, 398, FONT_NORMAL_WHITE);
	width += text_draw_number(Data_CityInfo.salaryAmount, '@', " ", 120 + width, 398, FONT_NORMAL_WHITE);
	lang_text_draw(52, 3, 120 + width, 398, FONT_NORMAL_WHITE);

	button_border_draw(320, 341, 250, 20, focusButtonId == 3);
	lang_text_draw_centered(52, 49, 320, 346, 250, FONT_NORMAL_WHITE);

	// Request buttons
	if (getRequestStatus(0)) {
		button_border_draw(38, 96, 560, 40, focusButtonId == 4);
	}
	if (getRequestStatus(1)) {
		button_border_draw(38, 138, 560, 40, focusButtonId == 5);
	}
	if (getRequestStatus(2)) {
		button_border_draw(38, 180, 560, 40, focusButtonId == 6);
	}
	if (getRequestStatus(3)) {
		button_border_draw(38, 222, 560, 40, focusButtonId == 7);
	}
	if (getRequestStatus(4)) {
		button_border_draw(38, 264, 560, 40, focusButtonId == 8);
	}
	graphics_reset_dialog();
}

static int getRequestStatus(int index)
{
	int numRequests = 0;
	if (Data_CityInfo.distantBattleMonthsToBattle > 0 && Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
		numRequests = 1;
		if (index == 0) {
			if (Data_CityInfo.militaryTotalLegions <= 0) {
				return -4;
			} else if (Data_CityInfo.militaryTotalLegionsEmpireService <= 0) {
				return -3;
			} else {
				return -2;
			}
		}
	}
    const scenario_request *request = scenario_request_get_visible(index - numRequests);
    if (request) {
        if (request->resource == RESOURCE_DENARII) {
            if (city_finance_treasury() <= request->amount) {
                return -1;
            }
        } else {
            if (Data_CityInfo.resourceStored[request->resource] < request->amount) {
                return -1;
            }
        }
        return request->id + 1;
    }
    return 0;
}

void UI_Advisor_Imperial_handleMouse(const mouse *m)
{
	generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, imperialButtons, 8, &focusButtonId);
}

static void buttonDonateToCity(int param1, int param2)
{
	UI_Window_goTo(Window_DonateToCityDialog);
}

static void buttonSetSalary(int param1, int param2)
{
	UI_Window_goTo(Window_SetSalaryDialog);
}

static void buttonGiftToCaesar(int param1, int param2)
{
	UI_Window_goTo(Window_SendGiftToCaesarDialog);
}

static void buttonRequest(int index, int param2)
{
	int status = getRequestStatus(index);
	if (status) {
		Data_CityInfo.militaryTotalLegionsEmpireService = 0;
		switch (status) {
			case -4:
				UI_PopupDialog_show(PopupDialog_NoLegionsAvailable, confirmNothing, 0);
				break;
			case -3:
				UI_PopupDialog_show(PopupDialog_NoLegionsSelected, confirmNothing, 0);
				break;
			case -2:
				UI_PopupDialog_show(PopupDialog_RequestSendTroops, confirmSendTroops, 2);
				break;
			case -1:
				UI_PopupDialog_show(PopupDialog_RequestNotEnoughGoods, confirmNothing, 0);
				break;
			default:
				selectedRequestId = status - 1;
				UI_PopupDialog_show(PopupDialog_RequestSendGoods, confirmSendGoods, 2);
				break;
		}
	}
}

static void confirmNothing(int accepted)
{
	// do nothing..
}

static void confirmSendTroops(int accepted)
{
	if (accepted) {
		formation_legions_dispatch_to_distant_battle();
		UI_Window_goTo(Window_Empire);
	}
}

static void confirmSendGoods(int accepted)
{
	if (accepted) {
		scenario_request_dispatch(selectedRequestId);
	}
}

int UI_Advisor_Imperial_getTooltip()
{
	if (focusButtonId && focusButtonId <= 2) {
		return 93 + focusButtonId;
	} else if (focusButtonId == 3) {
		return 131;
	} else {
		return 0;
	}
}
