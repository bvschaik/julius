#include "Window.h"
#include "AllWindows.h"
#include "PopupDialog.h"
#include "../Graphics.h"
#include "../Widget.h"
#include "../Empire.h"
#include "../Resource.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"
#include "../Util.h"
#include "../Calc.h"

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
static short getNextAnimationIndex(int graphicId, short currentAnimationIndex);
static int getSelectedObject();

static void buttonHelp(int param1, int param2);
static void buttonReturnToCity(int param1, int param2);
static void buttonAdvisor(int param1, int param2);
static void buttonOpenTrade(int param1, int param2);
static void buttonEmpireMap(int param1, int param2);
static void confirmOpenTrade(int accepted);

ImageButton imageButtonHelp[] = {
	{0, 0, 27, 27, 4, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0}
};
ImageButton imageButtonReturnToCity[] = {
	{0, 0, 24, 24, 4, 134, 4, buttonReturnToCity, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0}
};
ImageButton imageButtonAdvisor[] = {
	{-4, 0, 24, 24, 4, 199, 12, buttonAdvisor, Widget_Button_doNothing, 1, 0, 0, 0, 5, 0}
};
CustomButton customButtonOpenTrade[] = {
	{50, 68, 450, 91, buttonOpenTrade, Widget_Button_doNothing, 1, 0, 0}
};

ImageButton imageButtonsTradeOpened[] = {
	{92, 248, 28, 28, 4, 199, 12, buttonAdvisor, Widget_Button_doNothing, 1, 0, 0, 0, 5, 0},
	{522, 252, 24, 24, 4, 134, 4, buttonEmpireMap, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
};

static int selectedButton = 0;
static int selectedCity = 1;
static int xMin, xMax, yMin, yMax;

void UI_Empire_drawBackground()
{
	xMin = Data_Screen.width <= MAX_WIDTH ? 0 : (Data_Screen.width - MAX_WIDTH) / 2;
	xMax = Data_Screen.width <= MAX_WIDTH ? Data_Screen.width : xMin + MAX_WIDTH;
	yMin = Data_Screen.height <= MAX_HEIGHT ? 0 : (Data_Screen.height - MAX_HEIGHT) / 2;
	yMax = Data_Screen.height <= MAX_HEIGHT ? Data_Screen.height : yMin + MAX_HEIGHT;

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
	int graphicBase = GraphicId(ID_Graphic_EmpirePanels);
	// bottom panel background
	Graphics_setClipRectangle(xMin, yMin, xMax - xMin, yMax - yMin);
	for (int x = xMin; x < xMax; x += 70) {
		Graphics_drawImage(graphicBase + 3, x, yMax - 120);
		Graphics_drawImage(graphicBase + 3, x, yMax - 80);
		Graphics_drawImage(graphicBase + 3, x, yMax - 40);
	}
	//Graphics_drawImage(graphicBase + 3, xMax - 70, Data_Screen.height - 120);
	//Graphics_drawImage(graphicBase + 3, xMax - 70, Data_Screen.height - 80);
	//Graphics_drawImage(graphicBase + 3, xMax - 70, Data_Screen.height - 40);

	// horizontal bar borders
	for (int x = xMin; x < xMax; x += 86) {
		Graphics_drawImage(graphicBase + 1, x, yMin);
		Graphics_drawImage(graphicBase + 1, x, yMax - 120);
		Graphics_drawImage(graphicBase + 1, x, yMax - 16);
	}
	//Graphics_drawImage(graphicBase + 1, xMax - 86, yMin);
	//Graphics_drawImage(graphicBase + 1, xMax - 86, yMax - 120);
	//Graphics_drawImage(graphicBase + 1, xMax - 86, yMax - 16);

	// vertical bar borders
	for (int y = yMin + 16; y < yMax; y += 86) {
		Graphics_drawImage(graphicBase, xMin, y);
		Graphics_drawImage(graphicBase, xMax - 16, y);
	}
	//Graphics_drawImage(graphicBase, xMin, yMax - 86);
	//Graphics_drawImage(graphicBase, xMax - 16, yMax - 86);

	// crossbars
	Graphics_drawImage(graphicBase + 2, xMin, yMin);
	Graphics_drawImage(graphicBase + 2, xMin, yMax - 120);
	Graphics_drawImage(graphicBase + 2, xMin, yMax - 16);
	Graphics_drawImage(graphicBase + 2, xMax - 16, yMin);
	Graphics_drawImage(graphicBase + 2, xMax - 16, yMax - 120);
	Graphics_drawImage(graphicBase + 2, xMax - 16, yMax - 16);

	Graphics_resetClipRectangle();
}

void drawPanelInfo()
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
		Widget_GameText_drawCentered(47, 8, xMin, yMax - 48, xMax - xMin, Font_SmallBrown);
	}
}

static void drawPanelInfoCity()
{
	int objectId = Data_Empire.selectedObject - 1;
	int xOffset = (xMin + xMax - 240) / 2;
	int yOffset = yMax - 88;

	if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_DistantRoman) {
		Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, Font_SmallBrown);
		return;
	}
	if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_VulnerableRoman) {
		if (Data_CityInfo.distantBattleCityMonthsUntilRoman <= 0) {
			Widget_GameText_drawCentered(47, 12, xOffset, yOffset + 42, 240, Font_SmallBrown);
		} else {
			Widget_GameText_drawCentered(47, 13, xOffset, yOffset + 42, 240, Font_SmallBrown);
		}
		return;
	}
	if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_FutureTrade ||
		Data_Empire_Cities[selectedCity].cityType == EmpireCity_DistantForeign ||
		Data_Empire_Cities[selectedCity].cityType == EmpireCity_FutureRoman) {
		Widget_GameText_drawCentered(47, 0, xOffset, yOffset + 42, 240, Font_SmallBrown);
		return;
	}
	if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_Ours) {
		Widget_GameText_drawCentered(47, 1, xOffset, yOffset + 42, 240, Font_SmallBrown);
		return;
	}
	if (Data_Empire_Cities[selectedCity].cityType != EmpireCity_Trade) {
		return;
	}
	// trade city
	xOffset = (xMin + xMax - 500) / 2;
	yOffset = yMax - 108;
	Data_Empire_Cities[selectedCity].isOpen = 0;
	if (Data_Empire_Cities[selectedCity].isOpen) {
		Widget_GameText_draw(47, 10, xOffset + 40, yOffset + 30, Font_SmallBrown);
		int goodOffset = 0;
		for (int good = 1; good <= 15; good++) {
			if (!Empire_citySellsResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 21, 26, 26);
			int graphicId = good + GraphicId(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicId(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 22);
			int routeId = Data_Empire_Cities[selectedCity].routeId;
			switch (Data_Empire_Trade.maxPerYear[routeId][good]) {
				case 15:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount),
						xOffset + 100 * goodOffset + 141, yOffset + 20);
					break;
				case 25:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 1,
						xOffset + 100 * goodOffset + 137, yOffset + 20);
					break;
				case 40:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 2,
						xOffset + 100 * goodOffset + 133, yOffset + 20);
					break;
			}
			int tradeNow = Data_Empire_Trade.tradedThisYear[routeId][good];
			int tradeMax = Data_Empire_Trade.maxPerYear[routeId][good];
			if (tradeNow > tradeMax) {
				tradeMax = tradeNow;
			}
			int textWidth = Widget_Text_drawNumber(tradeNow, '@', "",
				xOffset + 100 * goodOffset + 150, yOffset + 30, Font_SmallBrown);
			textWidth += Widget_GameText_draw(47, 11,
				xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 30, Font_SmallBrown);
			Widget_Text_drawNumber(tradeMax, '@', "",
				xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 30, Font_SmallBrown);
			goodOffset++;
		}
		Widget_GameText_draw(47, 9, xOffset + 40, yOffset + 60, Font_SmallBrown);
		goodOffset = 0;
		for (int good = 1; good <= 15; good++) {
			if (!Empire_cityBuysResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 51, 26, 26);
			int graphicId = good + GraphicId(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicId(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + 100 * goodOffset + 121, yOffset + 52);
			int routeId = Data_Empire_Cities[selectedCity].routeId;
			switch (Data_Empire_Trade.maxPerYear[routeId][good]) {
				case 15:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount),
						xOffset + 100 * goodOffset + 141, yOffset + 50);
					break;
				case 25:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 1,
						xOffset + 100 * goodOffset + 137, yOffset + 50);
					break;
				case 40:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 2,
						xOffset + 100 * goodOffset + 133, yOffset + 50);
					break;
			}
			int tradeNow = Data_Empire_Trade.tradedThisYear[routeId][good];
			int tradeMax = Data_Empire_Trade.maxPerYear[routeId][good];
			if (tradeNow > tradeMax) {
				tradeMax = tradeNow;
			}
			int textWidth = Widget_Text_drawNumber(tradeNow, '@', "",
				xOffset + 100 * goodOffset + 150, yOffset + 60, Font_SmallBrown);
			textWidth += Widget_GameText_draw(47, 11,
				xOffset + 100 * goodOffset + 148 + textWidth, yOffset + 60, Font_SmallBrown);
			Widget_Text_drawNumber(tradeMax, '@', "",
				xOffset + 100 * goodOffset + 138 + textWidth, yOffset + 60, Font_SmallBrown);
			goodOffset++;
		}
	} else { // trade is closed
		int goodOffset = Widget_GameText_draw(47, 5, xOffset + 50, yOffset + 42, Font_SmallBrown);
		for (int good = 1; good <= 15; good++) {
			if (!Empire_citySellsResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + goodOffset + 60, yOffset + 33, 26, 26);
			int graphicId = good + GraphicId(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicId(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 61, yOffset + 34);
			int routeId = Data_Empire_Cities[selectedCity].routeId;
			switch (Data_Empire_Trade.maxPerYear[routeId][good]) {
				case 15:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount),
						xOffset + goodOffset + 81, yOffset + 32);
					break;
				case 25:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 1,
						xOffset + goodOffset + 77, yOffset + 32);
					break;
				case 40:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 2,
						xOffset + goodOffset + 73, yOffset + 32);
					break;
			}
			goodOffset += 32;
		}
		goodOffset += Widget_GameText_draw(47, 4, xOffset + goodOffset + 100, yOffset + 42, Font_SmallBrown);
		for (int good = 1; good <= 15; good++) {
			if (!Empire_cityBuysResource(objectId, good)) {
				continue;
			}
			Graphics_drawInsetRect(xOffset + goodOffset + 110, yOffset + 33, 26, 26);
			int graphicId = good + GraphicId(ID_Graphic_EmpireResource);
			int resourceOffset = Resource_getGraphicId(good, 3);
			Graphics_drawImage(graphicId + resourceOffset, xOffset + goodOffset + 110, yOffset + 34);
			int routeId = Data_Empire_Cities[selectedCity].routeId;
			switch (Data_Empire_Trade.maxPerYear[routeId][good]) {
				case 15:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount),
						xOffset + goodOffset + 130, yOffset + 32);
					break;
				case 25:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 1,
						xOffset + goodOffset + 126, yOffset + 32);
					break;
				case 40:
					Graphics_drawImage(GraphicId(ID_Graphic_TradeAmount) + 2,
						xOffset + goodOffset + 122, yOffset + 32);
					break;
			}
			goodOffset += 32;
		}
		Widget_Panel_drawButtonBorder(xOffset + 50, yOffset + 68, 400, 20, selectedButton);
		goodOffset = Widget_GameText_drawNumberWithDescription(8, 0,
			Data_Empire_Cities[selectedCity].costToOpen,
			xOffset + 60, yOffset + 73, Font_SmallBrown);
		Widget_GameText_draw(47, 6, xOffset + goodOffset + 60, yOffset + 73, Font_SmallBrown);
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
			Data_Empire_Objects[Data_Empire.selectedObject].distantBattleTravelMonths) {
			int xOffset = (xMin + xMax - 240) / 2;
			int yOffset = yMax - 88;
			int textId;
			if (Data_CityInfo.distantBattleRomanMonthsToTravel) {
				textId = 15;
			} else {
				textId = 16;
			}
			Widget_GameText_drawMultiline(47, textId, xOffset, yOffset, 240, Font_NormalBlack);
		}
	}
}

static void drawPanelInfoEnemyArmy()
{
	if (Data_CityInfo.distantBattleMonthsToBattle > 0) {
		if (Data_CityInfo.distantBattleEnemyMonthsTraveled ==
			Data_Empire_Objects[Data_Empire.selectedObject].distantBattleTravelMonths) {
			Widget_GameText_drawMultiline(47, 14,
				(xMin + xMax - 240) / 2,
				yMax - 68,
				240, Font_NormalBlack);
		}
	}
}

static void drawPanelInfoCityName()
{
	int graphicBase = GraphicId(ID_Graphic_EmpirePanels);
	Graphics_drawImage(graphicBase + 6, xMin + 2, yMax - 199);
	Graphics_drawImage(graphicBase + 7, xMax - 84, yMax - 199);
	Graphics_drawImage(graphicBase + 8, (xMin + xMax - 332) / 2, yMax - 181);
	if (Data_Empire.selectedObject > 0) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			Widget_GameText_drawCentered(21, Data_Empire_Cities[selectedCity].cityNameId,
				(xMin + xMax - 332) / 2 + 64, yMax - 118, 268, Font_LargeBlack);
		}
	}
}

static void drawPanelButtons()
{
	Widget_Button_drawImageButtons(xMin + 20, yMax - 44, imageButtonHelp, 1);
	Widget_Button_drawImageButtons(xMax - 44, yMax - 44, imageButtonReturnToCity, 1);
	Widget_Button_drawImageButtons(xMax - 44, yMax - 100, imageButtonAdvisor, 1);
	if (Data_Empire.selectedObject) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			selectedCity = Empire_getCityForObject(Data_Empire.selectedObject-1);
			if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_Trade && !Data_Empire_Cities[selectedCity].isOpen) {
				Widget_Panel_drawButtonBorder((xMin + xMax - 500) / 2 + 50, yMax - 40, 400, 20, selectedButton);
			}
		}
	}
}

static void drawEmpireMap()
{
	Graphics_setClipRectangle(xMin + 16, yMin + 16, xMax - xMin - 32, yMax - yMin - 136);

	BOUND(Data_Empire.scrollX, 0, 2000 - (xMax - xMin - 32));
	BOUND(Data_Empire.scrollY, 0, 1000 - (yMax - yMin - 136));

	int xOffset = xMin + 16 - Data_Empire.scrollX;
	int yOffset = yMin + 16 - Data_Empire.scrollY;
	Graphics_drawImage(GraphicId(ID_Graphic_EmpireMap), xOffset, yOffset);

	for (int i = 0; i < 200 && Data_Empire_Objects[i].inUse; i++) {
		Data_Empire_Object *obj = &Data_Empire_Objects[i];
		if (obj->type == EmpireObject_LandTradeRoute || obj->type == EmpireObject_SeaTradeRoute) {
			if (!Empire_isTradeWithCityOpen(obj->tradeRouteId)) {
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
				graphicId = GraphicId(ID_Graphic_EmpireForeignCity);
			}
		}
		if (obj->type == EmpireObject_BattleIcon) {
			continue; // ???
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
			if (Data_CityInfo.distantBattleRomanMonthsToTravel <= 0) {
				continue;
			}
			if (Data_CityInfo.distantBattleRomanMonthsTraveled != obj->distantBattleTravelMonths) {
				continue;
			}
		}
		Graphics_drawImage(graphicId, xOffset + x, yOffset + y);
		if (GraphicHasAnimationSprite(graphicId)) {
			obj->animationIndex = getNextAnimationIndex(graphicId, obj->animationIndex);
			Graphics_drawImage(graphicId + obj->animationIndex,
				xOffset + x + GraphicAnimationTopOffset(graphicId),
				yOffset + y + GraphicAnimationLeftOffset(graphicId));
		}
	}

	Graphics_resetClipRectangle();
}

static short getNextAnimationIndex(int graphicId, short currentAnimationIndex)
{
	// TODO animation timers
	return 1;
}

static int getSelectedObject()
{
	if (!Data_Mouse.isLeftClick) {
		return -1;
	}
	if (Data_Mouse.x < xMin + 16 || Data_Mouse.x >= xMax - 16 ||
		Data_Mouse.y < yMin + 16 || Data_Mouse.y >= yMax - 120) {
		return -1;
	}
	int xMap = Data_Mouse.x + Data_Empire.scrollX - xMin - 16;
	int yMap = Data_Mouse.y + Data_Empire.scrollY - yMin - 16;
	int minDist = 10000;
	int objId = 0;
	for (int i = 0; i < 200 && Data_Empire_Objects[i].inUse; i++) {
		Data_Empire_Object *obj = &Data_Empire_Objects[i];
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
		int dist = Calc_distanceMaximum(xMap, yMap,
			xObj + obj->width / 2, yObj + obj->height / 2);
		if (dist < minDist) {
			minDist = dist;
			objId = i + 1;
		}
	}
	return objId;
}

void UI_Empire_handleMouse()
{
	// TODO scrolling
	//j_fun_scrollEmpireMap(j_fun_getMapScrollDirection());
	if (Widget_Button_handleImageButtons(xMin + 20, yMax - 44, imageButtonHelp, 1)) {
		return;
	}
	if (Widget_Button_handleImageButtons(xMax - 44, yMax - 44, imageButtonReturnToCity, 1)) {
		return;
	}
	if (Widget_Button_handleImageButtons(xMax - 44, yMax - 100, imageButtonAdvisor, 1)) {
		return;
	}
	int objectId = getSelectedObject();
	if (objectId > 0) {
		Data_Empire.selectedObject = objectId;
	} else if (objectId == 0) {
		Data_Empire.selectedObject = 0;
	}
	if (Data_Mouse.isRightClick) {
		Data_Empire.selectedObject = 0;
		UI_Window_requestRefresh();
	}
	if (Data_Empire.selectedObject) {
		if (Data_Empire_Objects[Data_Empire.selectedObject-1].type == EmpireObject_City) {
			selectedCity = Empire_getCityForObject(Data_Empire.selectedObject-1);
			if (Data_Empire_Cities[selectedCity].cityType == EmpireCity_Trade && !Data_Empire_Cities[selectedCity].isOpen) {
				Widget_Button_handleCustomButtons((xMin + xMax - 500) / 2, yMax - 105, customButtonOpenTrade, 1, &selectedButton);
			}
		}
	}
}

static void buttonHelp(int param1, int param2)
{
	// TODO
	// UI_HelpDialog_show(32, 1);
}

static void buttonReturnToCity(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void buttonAdvisor(int advisor, int param2)
{
	// TODO consolidate all go-to-advisor actions into 1 function (just like original)
	UI_Advisors_setAdvisor(advisor);
	UI_Window_goTo(Window_Advisors);
}

static void buttonOpenTrade(int param1, int param2)
{
	UI_PopupDialog_show(2, confirmOpenTrade, 2);
}

static void buttonEmpireMap(int param1, int param2)
{
	UI_Window_goTo(Window_Empire);
}

static void confirmOpenTrade(int accepted)
{
	if (accepted) {
		// TODO
		//j_fun_spendMoneyConstruction(ciid, trade_costToOpen[33 * trade_selectedCity]);
		Data_Empire_Cities[selectedCity].isOpen = 1;
		//j_fun_enableBuildingMenuItems();
		//j_fun_enableSidebarButtons();
		UI_Window_goTo(Window_TradeOpenedDialog);
	}
}

void UI_TradeOpenedDialog_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(xOffset + 80, yOffset + 64, 30, 14);
	Widget_GameText_drawCentered(142, 0, xOffset + 80, yOffset + 80, 480, Font_LargeBlack);
	if (Data_Empire_Cities[selectedCity].isSeaTrade) {
		Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 120, 416, Font_NormalBlack);
		Widget_GameText_drawMultiline(142, 3, xOffset + 112, yOffset + 184, 416, Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(142, 1, xOffset + 112, yOffset + 152, 416, Font_NormalBlack);
	}
	Widget_GameText_draw(142, 2, xOffset + 128, yOffset + 256, Font_NormalBlack);
}

void UI_TradeOpenedDialog_drawForeground()
{
	Widget_Button_drawImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		imageButtonsTradeOpened, 2);
}

void UI_TradeOpenedDialog_handleMouse()
{
	Widget_Button_handleImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		imageButtonsTradeOpened, 2);
}
