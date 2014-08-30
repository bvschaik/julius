#include "AllWindows.h"
#include "Window.h"

#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

static int focusButtonId = 0;

void UI_MissionEnd_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;
	if (Data_State.winState != 1) {
		// lost mission
		Widget_Panel_drawOuterPanel(xOffset, yOffset - 112, 34, 16);
		Widget_GameText_drawCentered(62, 1, xOffset, yOffset - 96, 544, Font_LargeBlack);
		Widget_GameText_drawMultiline(62, 16, xOffset + 16, yOffset - 56, 496, Font_NormalBlack);
		return;
	}
	// won mission
	Widget_Panel_drawOuterPanel(xOffset, yOffset, 34, 18);
	Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, Font_LargeBlack);
	
	Widget_Panel_drawInnerPanel(xOffset + 16, yOffset + 56, 32, 7);

	if (Data_Settings.isCustomScenario) {
		Widget_GameText_drawMultiline(147, 20, xOffset + 32, yOffset + 64, 496, Font_NormalWhite);
	} else {
		Widget_GameText_drawMultiline(147, Data_Settings.saveGameMissionId, xOffset + 32, yOffset + 64, 496, Font_NormalWhite);
	}
	int width = Widget_GameText_draw(148, 0, xOffset + 40, yOffset + 180, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.ratingCulture, '@', " ",
		xOffset + 40 + width, yOffset + 180, Font_NormalBlack);

	width = Widget_GameText_draw(148, 1, xOffset + 300, yOffset + 180, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.ratingProsperity, '@', " ",
		xOffset + 300 + width, yOffset + 180, Font_NormalBlack);

	width = Widget_GameText_draw(148, 2, xOffset + 40, yOffset + 200, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.ratingPeace, '@', " ",
		xOffset + 40 + width, yOffset + 200, Font_NormalBlack);

	width = Widget_GameText_draw(148, 3, xOffset + 300, yOffset + 200, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.ratingFavor, '@', " ",
		xOffset + 300 + width, yOffset + 200, Font_NormalBlack);

	width = Widget_GameText_draw(148, 4, xOffset + 40, yOffset + 220, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.population, '@', " ",
		xOffset + 40 + width, yOffset + 220, Font_NormalBlack);

	width = Widget_GameText_draw(148, 5, xOffset + 300, yOffset + 220, Font_NormalBlack);
	Widget_Text_drawNumber(Data_CityInfo.treasury, '@', " ",
		xOffset + 300 + width, yOffset + 220, Font_NormalBlack);

	Widget_GameText_drawCentered(13, 1, xOffset + 16, yOffset + 260, 512, Font_NormalBlack);
}

void UI_MissionEnd_drawForeground()
{
	if (Data_State.winState != 1) {
		UI_VictoryDialog_drawForeground();
	}
}

void UI_MissionEnd_handleMouse()
{
	if (Data_Mouse.right.wentUp || Data_Mouse.left.wentDown) {
		UI_Window_goTo(Window_MainMenu);
	}
}

void UI_VictoryDialog_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;

	Widget_Panel_drawOuterPanel(xOffset, yOffset, 34, 15);
	if (Data_Settings.currentMissionId < 10 || Data_Settings.isCustomScenario) {
		Widget_GameText_drawCentered(62, 0, xOffset, yOffset + 16, 544, Font_LargeBlack);
		Widget_GameText_drawCentered(62, 2, xOffset, yOffset + 47, 544, Font_NormalBlack);
		Widget_GameText_drawCentered(32, Data_CityInfo.playerRank + 1, xOffset, yOffset + 66, 544, Font_LargeBlack);
	} else {
		Widget_Text_drawCentered(Data_Settings.playerName, xOffset, yOffset + 16, 512, Font_LargeBlack, 0);
		Widget_GameText_drawMultiline(62, 26, xOffset + 16, yOffset + 47, 480, Font_NormalBlack);
	}
}

void UI_VictoryDialog_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;

	if (Data_State.winState == 1) {
		Widget_Panel_drawLargeLabelButton(6, xOffset + 32, yOffset + 112, 30, focusButtonId == 1);
		if (Data_Settings.currentMissionId < 10 || Data_Settings.isCustomScenario) {
			Widget_GameText_drawCentered(62, 3,
				xOffset + 32, yOffset + 118, 480, Font_NormalGreen);
		} else {
			Widget_GameText_drawCentered(62, 27,
				xOffset + 32, yOffset + 118, 480, Font_NormalGreen);
		}
		if (Data_Settings.currentMissionId >= 2 || Data_Settings.isCustomScenario) {
			// Continue for 2/5 years
			Widget_Panel_drawLargeLabelButton(6, xOffset + 32, yOffset + 144, 30, focusButtonId == 2);
			Widget_GameText_drawCentered(62, 4, xOffset + 32, yOffset + 150, 480, Font_NormalGreen);

			Widget_Panel_drawLargeLabelButton(6, xOffset + 32, yOffset + 176, 30, focusButtonId == 3);
			Widget_GameText_drawCentered(62, 5, xOffset + 32, yOffset + 182, 480, Font_NormalGreen);
		}
	} else {
		// lost
		Widget_Panel_drawLargeLabelButton(6, xOffset + 32, yOffset + 96, 30, focusButtonId == 1);
		Widget_GameText_drawCentered(62, 6, xOffset + 32, yOffset + 102, 480, Font_NormalGreen);
	}
}

void UI_VictoryDialog_handleMouse()
{
	// TODO
}
