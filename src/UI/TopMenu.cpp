#include "TopMenu.h"
#include "Window.h"
#include "../Graphics.h"
#include "../Widget.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Screen.h"

static int offsetFunds;
static int offsetPopulation;
static int offsetDate;

static void drawBackground();
static void drawMenu();

void UI_TopMenu_draw()
{
	drawBackground();
	drawMenu();

	int width;
	Color treasureColor = Color_White;
	if (Data_CityInfo.treasury < 0) {
		treasureColor = Color_Red;
	}
	if (Data_Screen.width < 800) {
		offsetFunds = 338;
		offsetPopulation = 453;
		offsetDate = 547;
		
		width = Widget_GameText_draw(6, 0, 350, 5, Font_NormalPlain, treasureColor);
		Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ", 346 + width, 5, Font_NormalPlain, treasureColor);

		width = Widget_GameText_draw(6, 1, 458, 5, Font_SmallBrown, Color_White);
		Widget_Text_drawNumber(Data_CityInfo.population, '@', " ", 450 + width, 5, Font_SmallBrown, Color_White);

		width = Widget_GameText_draw(25, Data_CityInfo_Extra.gameTimeMonth, 552, 5, Font_SmallBrown, Color_Yellow);
		Widget_GameText_drawYearNoSpacing(Data_CityInfo_Extra.gameTimeYear, 541 + width, 5, Font_SmallBrown, Color_Yellow);
	} else if (Data_Screen.width < 1024) {
		offsetFunds = 338;
		offsetPopulation = 458;
		offsetDate = 652;
		
		width = Widget_GameText_draw(6, 0, 350, 5, Font_NormalPlain, treasureColor);
		Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ", 346 + width, 5, Font_NormalPlain, treasureColor);

		width = Widget_GameText_draw(6, 1, 470, 5, Font_NormalPlain, Color_White);
		Widget_Text_drawNumber(Data_CityInfo.population, '@', " ", 466 + width, 5, Font_NormalPlain, Color_White);

		width = Widget_GameText_draw(25, Data_CityInfo_Extra.gameTimeMonth, 655, 5, Font_NormalPlain, Color_Yellow);
		Widget_GameText_drawYear(Data_CityInfo_Extra.gameTimeYear, 655 + width, 5, Font_NormalPlain, Color_Yellow);
	} else {
		offsetFunds = 493;
		offsetPopulation = 637;
		offsetDate = 852;
		
		width = Widget_GameText_draw(6, 0, 495, 5, Font_NormalPlain, treasureColor);
		Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ", 501 + width, 5, Font_NormalPlain, treasureColor);

		width = Widget_GameText_draw(6, 1, 645, 5, Font_NormalPlain, Color_White);
		Widget_Text_drawNumber(Data_CityInfo.population, '@', " ", 651 + width, 5, Font_NormalPlain, Color_White);

		width = Widget_GameText_draw(25, Data_CityInfo_Extra.gameTimeMonth, 850, 5, Font_NormalPlain, Color_Yellow);
		Widget_GameText_drawYear(Data_CityInfo_Extra.gameTimeYear, 850 + width, 5, Font_NormalPlain, Color_Yellow);
	}
}

static void drawBackground()
{
	int blockWidth = 24;
	int graphicBase = GraphicId(ID_Graphic_TopMenuSidebar);
	for (int i = 0; i * blockWidth < Data_Screen.width; i++) {
		Graphics_drawImage(graphicBase + i % 8, i * blockWidth, 0);
	}
	// black panels for funds/pop/time
	if (Data_Screen.width < 800) {
		Graphics_drawImage(graphicBase + 14, 336, 0);
	} else if (Data_Screen.width < 1024) {
		Graphics_drawImage(graphicBase + 14, 336, 0);
		Graphics_drawImage(graphicBase + 14, 456, 0);
		Graphics_drawImage(graphicBase + 14, 648, 0);
	} else {
		Graphics_drawImage(graphicBase + 14, 480, 0);
		Graphics_drawImage(graphicBase + 14, 624, 0);
		Graphics_drawImage(graphicBase + 14, 840, 0);
	}
}

static void drawMenu()
{
}
