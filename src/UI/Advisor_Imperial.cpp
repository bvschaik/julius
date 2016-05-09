#include "Advisors_private.h"
#include "PopupDialog.h"
#include "Window.h"

#include "../CityInfo.h"
#include "../Event.h"
#include "../Formation.h"
#include "../Resource.h"

#include "../Data/Empire.h"
#include "../Data/Scenario.h"
#include "../Data/Settings.h"

static void buttonDonateToCity(int param1, int param2);
static void buttonSetSalary(int param1, int param2);
static void buttonGiftToCaesar(int param1, int param2);
static void buttonRequest(int param1, int param2);

static int getRequestStatus(int index);

static void confirmNothing(int accepted);
static void confirmSendTroops(int accepted);
static void confirmSendGoods(int accepted);

static CustomButton imperialButtons[] = {
	{320, 367, 570, 387, CustomButton_Immediate, buttonDonateToCity, Widget_Button_doNothing, 0, 0},
	{70, 393, 570, 413, CustomButton_Immediate, buttonSetSalary, Widget_Button_doNothing, 0, 0},
	{320, 341, 570, 361, CustomButton_Immediate, buttonGiftToCaesar, Widget_Button_doNothing, 0, 0},
	{38, 96, 598, 136, CustomButton_Immediate, buttonRequest, Widget_Button_doNothing, 0, 0},
	{38, 138, 598, 178, CustomButton_Immediate, buttonRequest, Widget_Button_doNothing, 1, 0},
	{38, 180, 598, 220, CustomButton_Immediate, buttonRequest, Widget_Button_doNothing, 2, 0},
	{38, 222, 598, 262, CustomButton_Immediate, buttonRequest, Widget_Button_doNothing, 3, 0},
	{38, 264, 598, 304, CustomButton_Immediate, buttonRequest, Widget_Button_doNothing, 4, 0},
};

static int focusButtonId;
static int selectedRequestId;

void UI_Advisor_Imperial_drawBackground(int *advisorHeight)
{
	CityInfo_Imperial_calculateGiftCosts();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 27;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 2, baseOffsetX + 10, baseOffsetY + 10);

	Widget_Text_draw(Data_Settings.playerName, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);

	int width = Widget_GameText_draw(52, 0, baseOffsetX + 60, baseOffsetY + 44, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.ratingFavor, '@', " ", baseOffsetX + 60 + width, baseOffsetY + 44, Font_NormalBlack);

	Widget_GameText_drawMultiline(52, Data_CityInfo.ratingFavor / 5 + 22,
		baseOffsetX + 60, baseOffsetY + 60, 544, Font_NormalBlack);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 90, 36, 14);
	
	int numRequests = 0;
	if (Data_CityInfo.distantBattleMonthsToBattle > 0 && Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
		// can send to distant battle
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96, 560, 40, 0);
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + Resource_Weapons,
			baseOffsetX + 50, baseOffsetY + 106);
		width = Widget_GameText_draw(52, 72, baseOffsetX + 80, baseOffsetY + 102, Font_NormalWhite);
		Widget_GameText_draw(21, Data_Empire_Cities[Data_CityInfo.distantBattleCityId].cityNameId,
			baseOffsetX + 50 + width, baseOffsetY + 102, Font_NormalWhite);
		int strengthTextId;
		if (Data_CityInfo.distantBattleEnemyStrength < 46) {
			strengthTextId = 73;
		} else if (Data_CityInfo.distantBattleEnemyStrength < 89) {
			strengthTextId = 74;
		} else {
			strengthTextId = 75;
		}
		width = Widget_GameText_draw(52, strengthTextId, baseOffsetX + 80, baseOffsetY + 120, Font_NormalWhite);
		Widget_GameText_drawNumberWithDescription(8, 4, Data_CityInfo.distantBattleMonthsToBattle,
			baseOffsetX + 80 + width, baseOffsetY + 120, Font_NormalWhite);
		numRequests = 1;
	}
	for (int i = 0; i < 20; i++) {
		if (Data_Scenario.requests.resourceId[i] && Data_Scenario.requests_isVisible[i] && numRequests < 5) {
			Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96 + 42 * numRequests, 560, 40, 0);
			Widget_Text_drawNumber(Data_Scenario.requests.amount[i], '@', " ",
				baseOffsetX + 40, baseOffsetY + 102 + 42 * numRequests, Font_NormalWhite);
			int resourceOffset = Data_Scenario.requests.resourceId[i] +
				Resource_getGraphicIdOffset(Data_Scenario.requests.resourceId[i], 3);
			Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + resourceOffset,
				baseOffsetX + 110, baseOffsetY + 100 + 42 * numRequests);
			Widget_GameText_draw(23, Data_Scenario.requests.resourceId[i],
				baseOffsetX + 150, baseOffsetY + 102 + 42 * numRequests, Font_NormalWhite);
			
			width = Widget_GameText_drawNumberWithDescription(8, 4, Data_Scenario.requests_monthsToComply[i],
				baseOffsetX + 310, baseOffsetY + 102 + 42 * numRequests, Font_NormalWhite);
			Widget_GameText_draw(12, 2, baseOffsetX + 310 + width, baseOffsetY + 102 + 42 * numRequests, Font_NormalWhite);

			if (Data_Scenario.requests.resourceId[i] == Resource_Denarii) {
				// request for money
				width = Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ",
					baseOffsetX + 40, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				width += Widget_GameText_draw(52, 44,
					baseOffsetX + 40 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				if (Data_CityInfo.treasury < Data_Scenario.requests.amount[i]) {
					Widget_GameText_draw(52, 48,
						baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				} else {
					Widget_GameText_draw(52, 47,
						baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				}
			} else {
				// normal goods request
				int resourceId = Data_Scenario.requests.resourceId[i];
				width = Widget_Text_drawNumber(Data_CityInfo.resourceStored[resourceId], '@', " ",
					baseOffsetX + 40, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				width += Widget_GameText_draw(52, 43,
					baseOffsetX + 40 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				if (Data_CityInfo.resourceStored[resourceId] < Data_Scenario.requests.amount[i]) {
					Widget_GameText_draw(52, 48,
						baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				} else {
					Widget_GameText_draw(52, 47,
						baseOffsetX + 80 + width, baseOffsetY + 120 + 42 * numRequests, Font_NormalWhite);
				}
			}
			numRequests++;
		}
	}
	if (!numRequests) {
		Widget_GameText_drawMultiline(52, 21, baseOffsetX + 64, baseOffsetY + 160, 512, Font_NormalWhite);
	}
}

void UI_Advisor_Imperial_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 324, 32, 6);

	Widget_GameText_draw(32, Data_CityInfo.playerRank,
		baseOffsetX + 72, baseOffsetY + 338, Font_LargeBrown);
	
	int width = Widget_GameText_draw(52, 1,
		baseOffsetX + 72, baseOffsetY + 372, Font_NormalWhite);
	Widget_Text_drawNumber(Data_CityInfo.personalSavings, '@', " Dn",
		baseOffsetX + 80 + width, baseOffsetY + 372, Font_NormalWhite);

	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 367,
		250, 20, focusButtonId == 1);
	Widget_GameText_drawCentered(52, 2, baseOffsetX + 320, baseOffsetY + 372, 250, Font_NormalWhite);

	Widget_Panel_drawButtonBorder(baseOffsetX + 70, baseOffsetY + 393,
		500, 20, focusButtonId == 2);
	width = Widget_GameText_draw(52, Data_CityInfo.salaryRank + 4,
		baseOffsetX + 120, baseOffsetY + 398, Font_NormalWhite);
	width += Widget_Text_drawNumber(Data_CityInfo.salaryAmount, '@', " ",
		baseOffsetX + 120 + width, baseOffsetY + 398, Font_NormalWhite);
	Widget_GameText_draw(52, 3, baseOffsetX + 120 + width, baseOffsetY + 398, Font_NormalWhite);

	Widget_Panel_drawButtonBorder(baseOffsetX + 320, baseOffsetY + 341,
		250, 20, focusButtonId == 3);
	Widget_GameText_drawCentered(52, 49, baseOffsetX + 320, baseOffsetY + 346, 250, Font_NormalWhite);

	// Request buttons
	if (getRequestStatus(0)) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 96,
			560, 40, focusButtonId == 4);
	}
	if (getRequestStatus(1)) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 138,
			560, 40, focusButtonId == 5);
	}
	if (getRequestStatus(2)) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 180,
			560, 40, focusButtonId == 6);
	}
	if (getRequestStatus(3)) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 222,
			560, 40, focusButtonId == 7);
	}
	if (getRequestStatus(4)) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 38, baseOffsetY + 264,
			560, 40, focusButtonId == 8);
	}
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
	for (int i = 0; i < 20; i++) {
		if (Data_Scenario.requests.resourceId[i] && Data_Scenario.requests_isVisible[i] &&
			Data_Scenario.requests_state[i] <= 1) {
			if (numRequests == index) {
				if (Data_Scenario.requests.resourceId[i] == Resource_Denarii) {
					if (Data_CityInfo.treasury <= Data_Scenario.requests.amount[i]) {
						return -1;
					}
				} else {
					int resourceId = Data_Scenario.requests.resourceId[i];
					if (Data_CityInfo.resourceStored[resourceId] < Data_Scenario.requests.amount[i]) {
						return -1;
					}
				}
				return i + 1;
			}
			numRequests++;
		}
	}
	return 0;
}

void UI_Advisor_Imperial_handleMouse()
{
	int offsetX = Data_Screen.offset640x480.x;
	int offsetY = Data_Screen.offset640x480.y;
	Widget_Button_handleCustomButtons(offsetX, offsetY,
		imperialButtons, 8, &focusButtonId);
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
		Formation_dispatchLegionsToDistantBattle();
		UI_Window_goTo(Window_Empire);
	}
}

static void confirmSendGoods(int accepted)
{
	if (accepted) {
		Event_dispatchRequest(selectedRequestId);
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
