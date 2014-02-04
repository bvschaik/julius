#include "Window.h"
#include "../Graphics.h"
#include "../Widget.h"
#include "../Empire.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"
#include "../Util.h"

#include <stdio.h>

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

static void drawPaneling();
static void drawPanelInfo();
static void drawPanelInfoCity();
static void drawPanelInfoBattleIcon();
static void drawPanelInfoRomanArmy();
static void drawPanelInfoEnemyArmy();
static void drawEmpireMap();
static short getNextAnimationIndex(int graphicId, short currentAnimationIndex);

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
		Widget_GameText_drawCentered(47, 8, xMin, yMax - 48, xMax - xMin, Font_NormalBlack);
	}
}

static void drawPanelInfoCity()
{
	int objectId = Data_Empire.selectedObject - 1;
	int xOffset = xMin + (xMax - xMin - 240) / 2;
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
	xOffset = xMin + (xMax - xMin - 500) / 2;
	yOffset = yMax - 108;
	if (Data_Empire_Cities[selectedCity].isOpen) {
		Widget_GameText_draw(47, 10, xOffset + 40, yOffset + 30, Font_SmallBrown);
		int goodOffset = 0;
		for (int good = 1; good <= 15; good++) {
			if (Empire_citySellsResource(objectId, good)) {
				Graphics_drawInsetRect(xOffset + 100 * goodOffset + 120, yOffset + 21, 26, 26);

			}
		}
	} else { // trade is closed
		
	}
}
/*
void __cdecl fun_drawEmpireInfoPanelCity()
{
    if ( trade_isOpen[66 * trade_selectedCity] )
    {
      v10 = 0;
      j_fun_drawGameText(47, 10, v26 + 40, v24 + 30, graphic_font + 1072, 0);
      for ( i = 1; i <= 15; ++i )
      {
        if ( j_fun_empireCitySellsGood(objectId, i) )
        {
          j_fun_drawInsetRect(v26 + 100 * v10 + 120, v24 + 21, 26, 26);
          v6 = i + word_6E6D0A;
          v7 = j_fun_getResourceGraphicIdOffset(i, 3);
          j_fun_drawGraphic(v7 + v6, v26 + 100 * v10 + 121, v24 + 22);
          switch ( *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + i) )
          {
            case 0xF:
              j_fun_drawGraphic(graphic_tradeAmount, v26 + 100 * v10 + 141, v24 + 20);
              break;
            case 0x19:
              j_fun_drawGraphic(graphic_tradeAmount + 1, v26 + 100 * v10 + 137, v24 + 20);
              break;
            case 0x28:
              j_fun_drawGraphic(graphic_tradeAmount + 2, v26 + 100 * v10 + 133, v24 + 20);
              break;
          }
          v20 = *(int *)((char *)&tradedSoFar[16 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]]
                       + 4 * i);
          v18 = *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + i);
          if ( v20 > v18 )
            v18 = *(int *)((char *)&tradedSoFar[16 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]]
                         + 4 * i);
          text_xoffset = 0;
          j_fun_drawNumber(v20, 64, byte_5E357C, v26 + 100 * v10 + 150, v24 + 30, graphic_font + 1072, 0);
          j_fun_drawGameText(47, 11, v26 + 100 * v10 + text_xoffset + 148, v24 + 30, graphic_font + 1072, 0);
          j_fun_drawNumber(
            v18,
            64,
            byte_5E357C,
            v26 + 100 * v10++ + text_xoffset + 138,
            v24 + 30,
            graphic_font + 1072,
            0);
        }
      }
      v11 = 0;
      j_fun_drawGameText(47, 9, v26 + 40, v24 + 60, graphic_font + 1072, 0);
      for ( j = 1; j <= 15; ++j )
      {
        if ( j_fun_empireCityBuysGood(objectId, j) )
        {
          j_fun_drawInsetRect(v26 + 100 * v11 + 120, v24 + 51, 26, 26);
          v8 = j + word_6E6D0A;
          v9 = j_fun_getResourceGraphicIdOffset(j, 3);
          j_fun_drawGraphic(v9 + v8, v26 + 100 * v11 + 121, v24 + 52);
          switch ( *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + j) )
          {
            case 0xF:
              j_fun_drawGraphic(graphic_tradeAmount, v26 + 100 * v11 + 141, v24 + 50);
              break;
            case 0x19:
              j_fun_drawGraphic(graphic_tradeAmount + 1, v26 + 100 * v11 + 137, v24 + 50);
              break;
            case 0x28:
              j_fun_drawGraphic(graphic_tradeAmount + 2, v26 + 100 * v11 + 133, v24 + 50);
              break;
          }
          v21 = *(int *)((char *)&tradedSoFar[16 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]]
                       + 4 * j);
          v19 = *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + j);
          if ( v21 > v19 )
            v19 = *(int *)((char *)&tradedSoFar[16 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]]
                         + 4 * j);
          text_xoffset = 0;
          j_fun_drawNumber(v21, 64, byte_5E357C, v26 + 100 * v11 + 150, v24 + 60, graphic_font + 1072, 0);
          j_fun_drawGameText(47, 11, v26 + 100 * v11 + text_xoffset + 148, v24 + 60, graphic_font + 1072, 0);
          j_fun_drawNumber(
            v19,
            64,
            byte_5E357C,
            v26 + 100 * v11++ + text_xoffset + 138,
            v24 + 60,
            graphic_font + 1072,
            0);
        }
      }
    }
    else
    {
      text_xoffset = 0;
      j_fun_drawGameText(47, 5, v26 + 50, v24 + 42, graphic_font + 1072, 0);
      v14 = 1;
      v12 = 0;
      while ( v14 <= 15 )
      {
        if ( j_fun_empireCitySellsGood(objectId, v14) )
        {
          j_fun_drawInsetRect(v26 + text_xoffset + 60, v24 + 33, 26, 26);
          v0 = v26 + text_xoffset + 61;
          v1 = v14 + word_6E6D0A;
          v2 = j_fun_getResourceGraphicIdOffset(v14, 3);
          j_fun_drawGraphic(v2 + v1, v0, v24 + 34);
          switch ( *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + v14) )
          {
            case 0xF:
              j_fun_drawGraphic(graphic_tradeAmount, v26 + text_xoffset + 81, v24 + 32);
              break;
            case 0x19:
              j_fun_drawGraphic(graphic_tradeAmount + 1, v26 + text_xoffset + 77, v24 + 32);
              break;
            case 0x28:
              j_fun_drawGraphic(graphic_tradeAmount + 2, v26 + text_xoffset + 73, v24 + 32);
              break;
          }
          text_xoffset += 32;
          ++v12;
        }
        ++v14;
      }
      j_fun_drawGameText(47, 4, v26 + text_xoffset + 100, v24 + 42, graphic_font + 1072, 0);
      v15 = 1;
      v13 = 0;
      while ( v15 <= 15 )
      {
        if ( j_fun_empireCityBuysGood(objectId, v15) )
        {
          j_fun_drawInsetRect(v26 + text_xoffset + 110, v24 + 33, 26, 26);
          v3 = v26 + text_xoffset + 110;
          v4 = v15 + word_6E6D0A;
          v5 = j_fun_getResourceGraphicIdOffset(v15, 3);
          j_fun_drawGraphic(v5 + v4, v3, v24 + 34);
          switch ( *((_DWORD *)&tradeQuotas[64 * (unsigned __int8)trade_routeId[66 * trade_selectedCity]] + v15) )
          {
            case 0xF:
              j_fun_drawGraphic(graphic_tradeAmount, v26 + text_xoffset + 130, v24 + 32);
              break;
            case 0x19:
              j_fun_drawGraphic(graphic_tradeAmount + 1, v26 + text_xoffset + 126, v24 + 32);
              break;
            case 0x28:
              j_fun_drawGraphic(graphic_tradeAmount + 2, v26 + text_xoffset + 122, v24 + 32);
              break;
          }
          text_xoffset += 32;
          ++v13;
        }
        ++v15;
      }
      if ( mouseover_button_id_main )
        j_fun_drawBorderedButton(0, v26 + 50, v24 + 68, 400, 20, 1);
      else
        j_fun_drawBorderedButton(0, v26 + 50, v24 + 68, 400, 20, 0);
      j_fun_drawNumberSingularPlural(
        8,
        0,
        trade_costToOpen[33 * trade_selectedCity],
        v26 + 60,
        v24 + 73,
        graphic_font + 1072,
        0);
      j_fun_drawGameText(47, 6, v26 + text_xoffset + 60, v24 + 73, graphic_font + 1072, 0);
    }
}
*/

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
			int xOffset = xMin + (xMax - xMin - 240) / 2;
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
				xMin + (xMax - xMin - 240) / 2,
				yMax - 68,
				240, Font_NormalBlack);
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

void UI_Empire_handleMouse()
{
	if (Data_Mouse.isRightClick) {
		UI_Window_goTo(Window_City);
	}
}
