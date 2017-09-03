#include "Window.h"
#include "AllWindows.h"
#include "Advisors.h"
#include "PopupDialog.h"
#include "MessageDialog.h"

#include "../Animation.h"
#include "core/calc.h"
#include "../CityInfo.h"
#include "../Empire.h"
#include "../Graphics.h"
#include "../Resource.h"
#include "../Scroll.h"
#include "../SidebarMenu.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Invasion.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"

#include "empire/trade_route.h"
#include "graphics/image.h"

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

static void drawPaneling();
static void drawPanelInfo();
static void drawPanelInfoCity();
static void drawPanelInfoBattleIcon();
static void drawPanelInfoRomanArmy();
static void drawPanelInfoEnemyArmy();
static void drawPanelInfoCityName();
static void drawPanelButtons();
static void drawEmpireMap();
static int getSelectedObject(const mouse *m);

static void buttonHelp(int param1, int param2);
static void buttonReturnToCity(int param1, int param2);
static void buttonAdvisor(int param1, int param2);
static void buttonOpenTrade(int param1, int param2);
static void buttonEmpireMap(int param1, int param2);
static void confirmOpenTrade(int accepted);

static ImageButton imageButtonHelp[] = {
	{0, 0, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget_Button_doNothing, 0, 0, 1}
};
static ImageButton imageButtonReturnToCity[] = {
	{0, 0, 24, 24, ImageButton_Normal, 134, 4, buttonReturnToCity, Widget_Button_doNothing, 0, 0, 1}
};
static ImageButton imageButtonAdvisor[] = {
	{-4, 0, 24, 24, ImageButton_Normal, 199, 12, buttonAdvisor, Widget_Button_doNothing, 5, 0, 1}
};
static CustomButton customButtonOpenTrade[] = {
	{50, 68, 450, 91, CustomButton_Immediate, buttonOpenTrade, Widget_Button_doNothing, 0, 0}
};

static ImageButton imageButtonsTradeOpened[] = {
	{92, 248, 28, 28, ImageButton_Normal, 199, 12, buttonAdvisor, Widget_Button_doNothing, 5, 0, 1},
	{522, 252, 24, 24, ImageButton_Normal, 134, 4, buttonEmpireMap, Widget_Button_doNothing, 0, 0, 1},
};

static struct {
	int selectedButton;
	int selectedCity;
	int xMin, xMax, yMin, yMax;
	int focusButtonId;
} data = {0, 1};

void UI_Empire_init()
{
	data.selectedButton = 0;
	if (Data_Empire.selectedObject) {
		data.selectedCity = Empire_getCityForObject(Data_Empire.selectedObject-1);
	} else {
		data.selectedCity = 0;
	}
	data.focusButtonId = 0;
}

void UI_Empire_drawBackground()
{
	data.xMin = Data_Screen.width <= MAX_WIDTH ? 0 : (Data_Screen.width - MAX_WIDTH) / 2;
	data.xMax = Data_Screen.width <= MAX_WIDTH ? Data_Screen.width : data.xMin + MAX_WIDTH;
	data.yMin = Data_Screen.height <= MAX_HEIGHT ? 0 : (Data_Screen.height - MAX_HEIGHT) / 2;
	data.yMax = Data_Screen.height <= MAX_HEIGHT ? Data_Screen.height : data.yMin + MAX_HEIGHT;

	if (data.xMin || data.yMin) {
		Graphics_clearScreen();
	}
	drawPaneling();
	drawPanelInfo();
}

void UI_Empire_drawForeground()
{
	drawEmpireMap();
	drawPanelInfoCityName();
	drawPanelButtons();
}

static void drawPaneling()
{
	int graphicBase = image_group(ID_Graphic_EmpirePanels);
	// bottom panel background
	Graphics_setClipRectangle(data.xMin, data.yMin, data.xMax - data.xMin, data.yMax - data.yMin);
	for (int x = data.xMin; x < data.xMax; x += 70) {
		Graphics_drawImage(graphicBase + 3, x, data.yMax - 120);
		Graphics_drawImage(graphicBase + 3, x, data.yMax - 80);
		Graphics_drawImage(graphicBase + 3, x, data.yMax - 40);
	}

	// horizontal bar borders
	for (int x = data.xMin; x < data.xMax; x += 86) {
		Graphics_drawImage(graphicBase + 1, x, data.yMin);
		Graphics_drawImage(graphicBase + 1, x, data.yMax - 120);
		Graphics_drawImage(graphicBase + 1, x, data.yMax - 16);
	}

	// vertical bar borders
	for (int y = data.yMin + 16; y < data.yMax; y += 86) {
		Graphics_drawImage(graphicBase, data.xMin, y);
		Graphics_drawImage(graphicBase, data.xMax - 16, y);
	}

	// crossbars
	Graphics_drawImage(graphicBase + 2, data.xMin, data.yMin);
	Graphics_drawImage(graphicBase + 2, data.xMin, data.yMax - 120);
	Graphics_drawImage(graphicBase + 2, data.xMin, data.yMax - 16);
	Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMin);
	Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMax - 120);
	Graphics_drawImage(graphicBase + 2, data.xMax - 16, data.yMax - 16);

	Graphics_resetClipRectangle();
}

static void drawPanelInfo()
{
	if (Data_Empire.selectedObject) {
		switch (Data_Empire_Objects[Data_Empire.selectedObject-1].type) {
			case EmpireObject_City:
				drawPanelInfoCity();
				break;
			case EmpireObject_BattleIcon:
				drawPanelInfoBattleIcon();
				break;
			case EmpireObject_RomanArmy:
				drawPanelInfoRomanArmy();
				break;
			case EmpireObject_EnemyArmy:
				drawPanelInfoEnemyArmy();
				break;
		}
	} else {
		Widget_GameText_drawCentered(47, 8, data.xMin, data.yMax - 48, data.xMax - data.xMin, FONT_NORMAL_GREEN);
	}
}

static void drawPanelInfoCity()
{
	int objectId = Data_Empire.selectedObject - 1;
	int xOffset = (data.xMin + data.xMax - 240) / 2;
	int yOffset = data.yMax - 88;

	if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_DistantRoman) {
		Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
		return;
	}
	if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_VulnerableRoman) {
		if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
		} else {
			Widget_GameText_drawCentered(47, 13, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
		}
		return;
	}
	if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_FutureTrade ||
		Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_DistantForeign ||
		Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_FutureRoman) {
		Widget_GameText_drawCentered(47, 0, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
		return;
	}
	if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_Ours) {
		Widget_GameText_drawCentered(47, 1, xOffset, yOffset + 42, 240, FONT_NORMAL_GREEN);
		return;
	}
	if (Data_Empire_Cities[data.selectedCity].cityType != EmpireCity_Trade) {
		return;
	}
	// trade city
	xOffset = (data.xMin + data.xMax - 500) / 2;
	yOffset = data.yMax - 108;
	if (Data_Empire_Cities[data.selectedCity].isOpen) {
		// city sells
		Widget_GameText_draw(47, 10, xOffset + 40, yOffset + 30, FONT_NORMAL_GREEN);
		int goodOffset = 0;
		for (int good = 1; good <= 15; good++) {
			if (!Empire_citySellsResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 21, 26, 26);
			int graphicId = good + image_group(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicIdOffset(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 22);
			int routeId = Data_Empire_Cities[data.selectedCity].routeId;
			int tradeMax = trade_route_limit(routeId, good);
			switch (tradeMax) {
				case 15:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount),
						xOffset + 100 * goodOffset + 141, yOffset + 20);
					break;
				case 25:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 1,
						xOffset + 100 * goodOffset + 137, yOffset + 20);
					break;
				case 40:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 2,
						xOffset + 100 * goodOffset + 133, yOffset + 20);
					break;
			}
			int tradeNow = trade_route_traded(routeId, good);
			if (tradeNow > tradeMax) {
				tradeMax = tradeNow;
			}
			int textWidth = Widget_Text_drawNumber(tradeNow, '@', "",
				xOffset + 100 * goodOffset + 150, yOffset + 30, FONT_NORMAL_GREEN);
			textWidth += Widget_GameText_draw(47, 11,
				xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 30, FONT_NORMAL_GREEN);
			Widget_Text_drawNumber(tradeMax, '@', "",
				xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 30, FONT_NORMAL_GREEN);
			goodOffset++;
		}
		// city buys
		Widget_GameText_draw(47, 9, xOffset + 40, yOffset + 60, FONT_NORMAL_GREEN);
		goodOffset = 0;
		for (int good = 1; good <= 15; good++) {
			if (!Empire_cityBuysResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 51, 26, 26);
			int graphicId = good + image_group(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicIdOffset(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 52);
			int routeId = Data_Empire_Cities[data.selectedCity].routeId;
			int tradeMax = trade_route_limit(routeId, good);
			switch (tradeMax) {
				case 15:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount),
						xOffset + 100 * goodOffset + 141, yOffset + 50);
					break;
				case 25:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 1,
						xOffset + 100 * goodOffset + 137, yOffset + 50);
					break;
				case 40:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 2,
						xOffset + 100 * goodOffset + 133, yOffset + 50);
					break;
			}
			int tradeNow = trade_route_traded(routeId, good);
			if (tradeNow > tradeMax) {
				tradeMax = tradeNow;
			}
			int textWidth = Widget_Text_drawNumber(tradeNow, '@', "",
				xOffset + 100 * goodOffset + 150, yOffset + 60, FONT_NORMAL_GREEN);
			textWidth += Widget_GameText_draw(47, 11,
				xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 60, FONT_NORMAL_GREEN);
			Widget_Text_drawNumber(tradeMax, '@', "",
				xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 60, FONT_NORMAL_GREEN);
			goodOffset++;
		}
	} else { // trade is closed
		int goodOffset = Widget_GameText_draw(47, 5, xOffset + 50, yOffset + 42, FONT_NORMAL_GREEN);
		for (int good = 1; good <= 15; good++) {
			if (!Empire_citySellsResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + goodOffset + 60, yOffset + 33, 26, 26);
			int graphicId = good + image_group(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicIdOffset(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 61, yOffset + 34);
			int routeId = Data_Empire_Cities[data.selectedCity].routeId;
			switch (trade_route_limit(routeId, good)) {
				case 15:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount),
						xOffset + goodOffset + 81, yOffset + 32);
					break;
				case 25:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 1,
						xOffset + goodOffset + 77, yOffset + 32);
					break;
				case 40:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 2,
						xOffset + goodOffset + 73, yOffset + 32);
					break;
			}
			goodOffset += 32;
		}
		goodOffset += Widget_GameText_draw(47, 4, xOffset + goodOffset + 100, yOffset + 42, FONT_NORMAL_GREEN);
		for (int good = 1; good <= 15; good++) {
			if (!Empire_cityBuysResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + goodOffset + 110, yOffset + 33, 26, 26);
			int graphicId = good + image_group(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicIdOffset(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 110, yOffset + 34);
			int routeId = Data_Empire_Cities[data.selectedCity].routeId;
			switch (trade_route_limit(routeId, good)) {
				case 15:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount),
						xOffset + goodOffset + 130, yOffset + 32);
					break;
				case 25:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 1,
						xOffset + goodOffset + 126, yOffset + 32);
					break;
				case 40:
					Graphics_drawImage(image_group(ID_Graphic_TradeAmount) + 2,
						xOffset + goodOffset + 122, yOffset + 32);
					break;
			}
			goodOffset += 32;
		}
		Widget_Panel_drawButtonBorder(xOffset + 50, yOffset + 68, 400, 20, data.selectedButton);
		goodOffset = Widget_GameText_drawNumberWithDescription(8, 0,
			Data_Empire_Cities[data.selectedCity].costToOpen,
			xOffset + 60, yOffset + 73, FONT_NORMAL_GREEN);
		Widget_GameText_draw(47, 6, xOffset + goodOffset + 60, yOffset + 73, FONT_NORMAL_GREEN);
	}
}

static void drawPanelInfoBattleIcon()
{
	// nothing
}

static void drawPanelInfoRomanArmy()
{
	if (Data_CityInfo.distantBattleRomanMonthsToTravel > 0 ||
		Data_CityInfo.distantBattleRomanMonthsToReturn > 0) {
		if (Data_CityInfo.distantBattleRomanMonthsTraveled ==
			Data_Empire_Objects[Data_Empire.selectedObject-1].distantBattleTravelMonths) {
			int xOffset = (data.xMin + data.xMax - 240) / 2;
			int yOffset = data.yMax - 88;
			int textId;
			if (Data_CityInfo.distantBattleRomanMonthsToTravel) {
				textId = 15;
			} else {
				textId = 16;
			}
			Widget_GameText_drawMultiline(47, textId, xOffset, yOffset, 240, FONT_NORMAL_BLACK);
		}
	}
}

static void drawPanelInfoEnemyArmy()
{
	if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
		if (Data_CityInfo.distantBattleEnemyMonthsTraveled ==
			Data_Empire_Objects[Data_Empire.selectedObject-1].distantBattleTravelMonths) {
			Widget_GameText_drawMultiline(47, 14,
				(data.xMin + data.xMax - 240) / 2,
				data.yMax - 68,
				240, FONT_NORMAL_BLACK);
		}
	}
}

static void drawPanelInfoCityName()
{
	int graphicBase = image_group(ID_Graphic_EmpirePanels);
	Graphics_drawImage(graphicBase + 6, data.xMin + 2, data.yMax - 199);
	Graphics_drawImage(graphicBase + 7, data.xMax - 84, data.yMax - 199);
	Graphics_drawImage(graphicBase + 8, (data.xMin + data.xMax - 332) / 2, data.yMax - 181);
	if (Data_Empire.selectedObject > 0) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			Widget_GameText_drawCentered(21, Data_Empire_Cities[data.selectedCity].cityNameId,
				(data.xMin + data.xMax - 332) / 2 + 64, data.yMax - 118, 268, FONT_LARGE_BLACK);
		}
	}
}

static void drawPanelButtons()
{
	Widget_Button_drawImageButtons(data.xMin + 20, data.yMax - 44, imageButtonHelp, 1);
	Widget_Button_drawImageButtons(data.xMax - 44, data.yMax - 44, imageButtonReturnToCity, 1);
	Widget_Button_drawImageButtons(data.xMax - 44, data.yMax - 100, imageButtonAdvisor, 1);
	if (Data_Empire.selectedObject) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			data.selectedCity = Empire_getCityForObject(Data_Empire.selectedObject-1);
			if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_Trade && !Data_Empire_Cities[data.selectedCity].isOpen) {
				Widget_Panel_drawButtonBorder((data.xMin + data.xMax - 500) / 2 + 50, data.yMax - 40, 400, 20, data.selectedButton);
			}
		}
	}
}

static void drawEmpireMap()
{
	Graphics_setClipRectangle(data.xMin + 16, data.yMin + 16, data.xMax - data.xMin - 32, data.yMax - data.yMin - 136);

	Data_Empire.scrollX = calc_bound(Data_Empire.scrollX, 0, 2000 - (data.xMax - data.xMin - 32));
	Data_Empire.scrollY = calc_bound(Data_Empire.scrollY, 0, 1000 - (data.yMax - data.yMin - 136));

	int xOffset = data.xMin + 16 - Data_Empire.scrollX;
	int yOffset = data.yMin + 16 - Data_Empire.scrollY;
	Graphics_drawImage(image_group(ID_Graphic_EmpireMap), xOffset, yOffset);

	for (int i = 0; i < 200 && Data_Empire_Objects[i].inUse; i++) {
		struct Data_Empire_Object *obj = &Data_Empire_Objects[i];
		if (obj->type == EmpireObject_LandTradeRoute || obj->type == EmpireObject_SeaTradeRoute) {
			if (!Empire_isTradeRouteOpen(obj->tradeRouteId)) {
				continue;
			}
		}
		int x, y, graphicId;
		if (Data_Scenario.empireHasExpanded) {
			x = obj->xExpanded;
			y = obj->yExpanded;
			graphicId = obj->graphicIdExpanded;
		} else {
			x = obj->x;
			y = obj->y;
			graphicId = obj->graphicId;
		}

		if (obj->type == EmpireObject_City) {
			int city = Empire_getCityForObject(i);
			if (Data_Empire_Cities[city].cityType == EmpireCity_DistantForeign ||
				Data_Empire_Cities[city].cityType == EmpireCity_FutureRoman) {
				graphicId = image_group(ID_Graphic_EmpireForeignCity);
			}
		}
		if (obj->type == EmpireObject_BattleIcon) {
			// handled below
			continue;
		}
		if (obj->type == EmpireObject_EnemyArmy) {
			if (Data_CityInfo.distantBattleMonthsToBattle <= 0) {
				continue;
			}
			if (Data_CityInfo.distantBattleEnemyMonthsTraveled != obj->distantBattleTravelMonths) {
				continue;
			}
		}
		if (obj->type == EmpireObject_RomanArmy) {
			if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0 &&
				Data_CityInfo.distantBattleRomanMonthsToReturn <= 0) {
				continue;
			}
			if (Data_CityInfo.distantBattleRomanMonthsTraveled != obj->distantBattleTravelMonths) {
				continue;
			}
		}
		Graphics_drawImage(graphicId, xOffset + x, yOffset + y);
		const image *img = image_get(graphicId);
		if (img->animation_speed_id) {
			obj->animationIndex = Animation_getIndexForEmpireMap(graphicId, obj->animationIndex);
			Graphics_drawImage(graphicId + obj->animationIndex,
				xOffset + x + img->sprite_offset_x,
				yOffset + y + img->sprite_offset_y);
		}
	}

	for (int i = 0; i < 101; i++) {
		if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
			Graphics_drawImage(Data_InvasionWarnings[i].empireGraphicId,
				xOffset + Data_InvasionWarnings[i].empireX,
				yOffset + Data_InvasionWarnings[i].empireY);
		}
	}
	Graphics_resetClipRectangle();
}

static int getSelectedObject(const mouse *m)
{
	if (!m->left.went_down) {
		return -1;
	}
	if (m->x < data.xMin + 16 || m->x >= data.xMax - 16 ||
		m->y < data.yMin + 16 || m->y >= data.yMax - 120) {
		return -1;
	}
	int xMap = m->x + Data_Empire.scrollX - data.xMin - 16;
	int yMap = m->y + Data_Empire.scrollY - data.yMin - 16;
	int minDist = 10000;
	int objId = 0;
	for (int i = 0; i < 200 && Data_Empire_Objects[i].inUse; i++) {
		struct Data_Empire_Object *obj = &Data_Empire_Objects[i];
		int xObj, yObj;
		if (Data_Scenario.empireHasExpanded) {
			xObj = obj->xExpanded;
			yObj = obj->yExpanded;
		} else {
			xObj = obj->x;
			yObj = obj->y;
		}
		if (xObj - 8 > xMap || xObj + obj->width + 8 <= xMap) {
			continue;
		}
		if (yObj - 8 > yMap || yObj + obj->height + 8 <= yMap) {
			continue;
		}
		int dist = calc_maximum_distance(xMap, yMap,
			xObj + obj->width / 2, yObj + obj->height / 2);
		if (dist < minDist) {
			minDist = dist;
			objId = i + 1;
		}
	}
	UI_Window_requestRefresh();
	return objId;
}

void UI_Empire_handleMouse(const mouse *m)
{
	Empire_scrollMap(Scroll_getDirection(m));
	data.focusButtonId = 0;
	int buttonId;
	Widget_Button_handleImageButtons(data.xMin + 20, data.yMax - 44, imageButtonHelp, 1, &buttonId);
	if (buttonId) {
		data.focusButtonId = 1;
	}
	Widget_Button_handleImageButtons(data.xMax - 44, data.yMax - 44, imageButtonReturnToCity, 1, &buttonId);
	if (buttonId) {
		data.focusButtonId = 2;
	}
	Widget_Button_handleImageButtons(data.xMax - 44, data.yMax - 100, imageButtonAdvisor, 1, &buttonId);
	if (buttonId) {
		data.focusButtonId = 3;
	}
	if (data.focusButtonId) {
		return;
	}
	int objectId = getSelectedObject(m);
	if (objectId > 0) {
		Data_Empire.selectedObject = objectId;
	} else if (objectId == 0) {
		Data_Empire.selectedObject = 0;
	}
	if (m->right.went_down) {
		Data_Empire.selectedObject = 0;
		UI_Window_requestRefresh();
	}
	if (Data_Empire.selectedObject) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			data.selectedCity = Empire_getCityForObject(Data_Empire.selectedObject-1);
			if (Data_Empire_Cities[data.selectedCity].cityType == EmpireCity_Trade && !Data_Empire_Cities[data.selectedCity].isOpen) {
				Widget_Button_handleCustomButtons((data.xMin + data.xMax - 500) / 2, data.yMax - 105, customButtonOpenTrade, 1, &data.selectedButton);
			}
		}
	}
}

static int isMouseHit(const mouse *m, int x, int y, int size)
{
    int mx = m->x;
    int my = m->y;
    return x <= mx && mx < x + size && y <= my && my < y + size;
}

static int getTooltipResource(const mouse *m)
{
	if (Data_Empire_Cities[data.selectedCity].cityType != EmpireCity_Trade) {
		return 0;
	}
	int objectId = Data_Empire.selectedObject - 1;
	int xOffset = (data.xMin + data.xMax - 500) / 2;
	int yOffset = data.yMax - 108;

	if (Data_Empire_Cities[data.selectedCity].isOpen) {
		for (int r = 1, index = 0; r <= 15; r++) {
			if (Empire_citySellsResource(objectId, r)) {
				if (isMouseHit(m, xOffset + 120 + 100 * index, yOffset + 21, 26)) {
					return r;
				}
				index++;
			}
		}
		for (int r = 1, index = 0; r <= 15; r++) {
			if (Empire_cityBuysResource(objectId, r)) {
				if (isMouseHit(m, xOffset + 120 + 100 * index, yOffset + 51, 26)) {
					return r;
				}
				index++;
			}
		}
	} else {
		int itemOffset = Widget_GameText_getDrawWidth(47, 5, FONT_NORMAL_GREEN);
		for (int r = 1; r <= 15; r++) {
			if (Empire_citySellsResource(objectId, r)) {
				if (isMouseHit(m, xOffset + 60 + itemOffset, yOffset + 35, 26)) {
					return r;
				}
				itemOffset += 32;
			}
		}
		itemOffset += Widget_GameText_getDrawWidth(47, 4, FONT_NORMAL_GREEN);
		for (int r = 1; r <= 15; r++) {
			if (Empire_cityBuysResource(objectId, r)) {
				if (isMouseHit(m, xOffset + 110 + itemOffset, yOffset + 35, 26)) {
					return r;
				}
				itemOffset += 32;
			}
		}
	}
	return 0;
}

void UI_EmpireMap_getTooltip(struct TooltipContext *c)
{
	int resource = getTooltipResource(mouse_get());
	if (resource) {
		c->type = TooltipType_Button;
		c->textId = 131 + resource;
	} else if (data.focusButtonId) {
		c->type = TooltipType_Button;
		switch (data.focusButtonId) {
			case 1: c->textId = 1; break;
			case 2: c->textId = 2; break;
			case 3: c->textId = 69; break;
		}
	}
}

static void buttonHelp(int param1, int param2)
{
	UI_MessageDialog_show(MessageDialog_EmpireMap, 1);
}

static void buttonReturnToCity(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void buttonAdvisor(int advisor, int param2)
{
	UI_Advisors_goToFromMessage(Advisor_Trade);
}

static void buttonOpenTrade(int param1, int param2)
{
	UI_PopupDialog_show(PopupDialog_OpenTrade, confirmOpenTrade, 2);
}

static void buttonEmpireMap(int param1, int param2)
{
	UI_Window_goTo(Window_Empire);
}

static void confirmOpenTrade(int accepted)
{
	if (accepted) {
		CityInfo_Finance_spendOnConstruction(Data_Empire_Cities[data.selectedCity].costToOpen);
		Data_Empire_Cities[data.selectedCity].isOpen = 1;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
		UI_Window_goTo(Window_TradeOpenedDialog);
	}
}

void UI_TradeOpenedDialog_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(xOffset + 80, yOffset + 64, 30, 14);
	Widget_GameText_drawCentered(142, 0, xOffset + 80, yOffset + 80, 480, FONT_LARGE_BLACK);
	if (Data_Empire_Cities[data.selectedCity].isSeaTrade) {
		Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 120, 416, FONT_NORMAL_BLACK);
		Widget_GameText_drawMultiline(142, 3, xOffset + 112, yOffset + 184, 416, FONT_NORMAL_BLACK);
	} else {
		Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 152, 416, FONT_NORMAL_BLACK);
	}
	Widget_GameText_draw(142, 2, xOffset + 128, yOffset + 256, FONT_NORMAL_BLACK);
}

void UI_TradeOpenedDialog_drawForeground()
{
	Widget_Button_drawImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		imageButtonsTradeOpened, 2);
}

void UI_TradeOpenedDialog_handleMouse(const mouse *m)
{
	Widget_Button_handleImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		imageButtonsTradeOpened, 2, 0);
}
