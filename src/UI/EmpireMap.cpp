#include "Window.h"
#include "../Graphics.h"
#include "../Widget.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
//#include "../Data/State.h"

static void drawPaneling();
static void drawPanelInfo();
static void drawPanelInfoCity();
static void drawPanelInfoBattleIcon();
static void drawPanelInfoRomanArmy();
static void drawPanelInfoEnemyArmy();


void UI_Empire_drawBackground()
{
	drawPaneling();
}

static void drawPaneling()
{
	int graphicBase = GraphicId(ID_Graphic_EmpirePanels);
	// bottom panel background
	for (int x = 0; x < Data_Screen.width; x += 70) {
		Graphics_drawImage(graphicBase + 3, x, Data_Screen.height - 120);
		Graphics_drawImage(graphicBase + 3, x, Data_Screen.height - 80);
		Graphics_drawImage(graphicBase + 3, x, Data_Screen.height - 40);
	}
	//Graphics_drawImage(graphicBase + 3, Data_Screen.width - 70, Data_Screen.height - 120);
	//Graphics_drawImage(graphicBase + 3, Data_Screen.width - 70, Data_Screen.height - 80);
	//Graphics_drawImage(graphicBase + 3, Data_Screen.width - 70, Data_Screen.height - 40);

	// horizontal bar borders
	for (int x = 0; x < Data_Screen.width; x += 86) {
		Graphics_drawImage(graphicBase + 1, x, 0);
		Graphics_drawImage(graphicBase + 1, x, Data_Screen.height - 120);
		Graphics_drawImage(graphicBase + 1, x, Data_Screen.height - 16);
	}
	//Graphics_drawImage(graphicBase + 1, Data_Screen.width - 86, 0);
	//Graphics_drawImage(graphicBase + 1, Data_Screen.width - 86, Data_Screen.height - 120);
	//Graphics_drawImage(graphicBase + 1, Data_Screen.width - 86, Data_Screen.height - 16);

	// vertical bar borders
	for (int y = 16; y < Data_Screen.height; y += 86) {
		Graphics_drawImage(graphicBase, 0, y);
		Graphics_drawImage(graphicBase, Data_Screen.width - 16, y);
	}
	//Graphics_drawImage(graphicBase, 0, Data_Screen.height - 86);
	//Graphics_drawImage(graphicBase, Data_Screen.width - 16, Data_Screen.height - 86);

	// crossbars
	Graphics_drawImage(graphicBase + 2, 0, 0);
	Graphics_drawImage(graphicBase + 2, 0, Data_Screen.height - 120);
	Graphics_drawImage(graphicBase + 2, 0, Data_Screen.height - 16);
	Graphics_drawImage(graphicBase + 2, Data_Screen.width - 16, 0);
	Graphics_drawImage(graphicBase + 2, Data_Screen.width - 16, Data_Screen.height - 120);
	Graphics_drawImage(graphicBase + 2, Data_Screen.width - 16, Data_Screen.height - 16);
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
		Widget_GameText_drawCentered(47, 8, 0, Data_Screen.height - 48, Data_Screen.width, Font_NormalBlack);
	}
}

static void drawPanelInfoCity()
{

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
			int xOffset = (Data_Screen.width - 240) / 2;
			int yOffset = Data_Screen.height - 88;
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
				(Data_Screen.width - 240) / 2,
				Data_Screen.height - 68,
				240, Font_NormalBlack);
		}
	}
}


void UI_Empire_handleMouse()
{
	if (Data_Mouse.isRightClick) {
		UI_Window_goTo(Window_City);
	}
}
