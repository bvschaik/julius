#include "Window.h"

#include "../Calc.h"
#include "../FileSystem.h"
#include "../GameFile.h"
#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/FileList.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"

#include <string.h>

static void drawScenarioList();
static void drawScrollbarDot();
static void drawScenarioInfo();
static int handleScrollbarClick();
static void buttonSelectItem(int param1, int param2);
static void buttonScroll(int param1, int param2);
static void buttonStartScenario(int param1, int param2);

static ImageButton imageButtons[] = {
	{276, 210, 34, 34, 4, 96, 8, buttonScroll, Widget_Button_doNothing, 1, 0, 0, 0, 0, 1},
	{276, 440, 34, 34, 4, 96, 12, buttonScroll, Widget_Button_doNothing, 1, 0, 0, 0, 1, 1},
	{600, 440, 27, 27, 4, 92, 56, buttonStartScenario, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0},
};

static CustomButton customButtons[] = {
	{28, 220, 280, 236, buttonSelectItem, Widget_Button_doNothing, 1, 0, 0},
	{28, 236, 280, 252, buttonSelectItem, Widget_Button_doNothing, 1, 1, 0},
	{28, 252, 280, 268, buttonSelectItem, Widget_Button_doNothing, 1, 2, 0},
	{28, 268, 280, 284, buttonSelectItem, Widget_Button_doNothing, 1, 3, 0},
	{28, 284, 280, 300, buttonSelectItem, Widget_Button_doNothing, 1, 4, 0},
	{28, 300, 280, 316, buttonSelectItem, Widget_Button_doNothing, 1, 5, 0},
	{28, 316, 280, 332, buttonSelectItem, Widget_Button_doNothing, 1, 6, 0},
	{28, 332, 280, 348, buttonSelectItem, Widget_Button_doNothing, 1, 7, 0},
	{28, 348, 280, 364, buttonSelectItem, Widget_Button_doNothing, 1, 8, 0},
	{28, 364, 280, 380, buttonSelectItem, Widget_Button_doNothing, 1, 9, 0},
	{28, 380, 280, 396, buttonSelectItem, Widget_Button_doNothing, 1, 10, 0},
	{28, 396, 280, 412, buttonSelectItem, Widget_Button_doNothing, 1, 11, 0},
	{28, 412, 280, 428, buttonSelectItem, Widget_Button_doNothing, 1, 12, 0},
	{28, 428, 280, 444, buttonSelectItem, Widget_Button_doNothing, 1, 13, 0},
	{28, 444, 280, 460, buttonSelectItem, Widget_Button_doNothing, 1, 14, 0},
};

static int scrollPosition;
static int focusButtonId;
static int selectedItem;

void UI_CCKSelection_init()
{
	FileSystem_findFilesWithExtension("map");
	scrollPosition = 0;
	focusButtonId = 0;
	buttonSelectItem(0, 0);
}

void UI_CCKSelection_drawBackground()
{
	Graphics_clearScreen();
	Graphics_drawImage(GraphicId(ID_Graphic_CCKBackground),
		(Data_Screen.width - 1024) / 2, (Data_Screen.height - 768) / 2);
	Widget_Panel_drawInnerPanel(
		Data_Screen.offset640x480.x + 280,
		Data_Screen.offset640x480.y + 242, 2, 12);
	drawScenarioList();
	drawScrollbarDot();
	drawScenarioInfo();
}

static void drawScenarioList()
{
	Widget_Panel_drawInnerPanel(Data_Screen.offset640x480.x + 16,
		Data_Screen.offset640x480.y + 210, 16, 16);
	for (int i = 0; i < 15; i++) {
		Font font = Font_NormalWhite;
		if (focusButtonId == i + 1) {
			font = Font_NormalRed;
		} else if (!focusButtonId && selectedItem == i + scrollPosition) {
			font = Font_NormalRed;
		}
		char file[100];
		strcpy(file, Data_FileList.files[i + scrollPosition]);
		FileSystem_removeExtension(file);
		Widget_Text_draw(file, Data_Screen.offset640x480.x + 24,
			Data_Screen.offset640x480.y + 220 + 16 * i, font, 0);
	}
}

static void drawScrollbarDot()
{
	if (Data_FileList.numFiles > 15) {
		int pct;
		if (scrollPosition <= 0) {
			pct = 0;
		} else if (scrollPosition + 15 >= Data_FileList.numFiles) {
			pct = 100;
		} else {
			pct = Calc_getPercentage(scrollPosition, Data_FileList.numFiles - 15);
		}
		int yOffset = Calc_adjustWithPercentage(164, pct);
		Graphics_drawImage(GraphicId(ID_Graphic_PanelButton) + 39,
			Data_Screen.offset640x480.x + 284,
			Data_Screen.offset640x480.y + 245 + yOffset);
	}
}

static void drawScenarioInfo()
{
	int baseOffsetX = Data_Screen.offset640x480.x + 320;
	int baseOffsetY = Data_Screen.offset640x480.y + 20;

	Graphics_drawImage(GraphicId(ID_Graphic_ScenarioImage) + Data_Scenario.imageId,
		Data_Screen.offset640x480.x + 78, Data_Screen.offset640x480.y + 36);

	Widget_Text_drawCentered(Data_FileList.selectedScenario,
		baseOffsetX + 15, baseOffsetY + 5, 260, Font_LargeBlack, 0);
	Widget_Text_drawCentered(Data_Scenario.briefDescription,
		baseOffsetX + 15, baseOffsetY + 40, 260, Font_NormalWhite, 0);
	Widget_GameText_drawYear(Data_Scenario.startYear,
		baseOffsetX + 90, baseOffsetY + 70, Font_LargeBlack);
	Widget_GameText_drawCentered(44, 77 + Data_Scenario.climate,
		baseOffsetX + 15, baseOffsetY + 130, 260, Font_NormalBlack);

	// map size
	int textId;
	switch (Data_Scenario.mapSizeX) {
		case 40: textId = 121; break;
		case 60: textId = 122; break;
		case 80: textId = 123; break;
		case 100: textId = 124; break;
		case 120: textId = 125; break;
		default: textId = 126; break;
	}
	Widget_GameText_drawCentered(44, textId,
		baseOffsetX + 15, baseOffsetY + 150, 260, Font_NormalBlack);

	// military
	int numInvasions = 0;
	for (int i = 0; i < 20; i++) {
		if (Data_Scenario.invasions.type[i]) {
			numInvasions++;
		}
	}
	if (numInvasions <= 0) {
		textId = 112;
	} else if (numInvasions <= 2) {
		textId = 113;
	} else if (numInvasions <= 4) {
		textId = 114;
	} else if (numInvasions <= 10) {
		textId = 115;
	} else {
		textId = 116;
	}
	Widget_GameText_drawCentered(44, textId,
		baseOffsetX + 15, baseOffsetY + 170, 260, Font_NormalBlack);

	Widget_GameText_drawCentered(32, 11 + Data_Scenario.playerRank,
		baseOffsetX + 15, baseOffsetY + 190, 260, Font_NormalBlack);
	if (Data_Scenario.isOpenPlay) {
		Widget_GameText_drawMultiline(145, Data_Scenario.openPlayScenarioId,
			baseOffsetX + 25, baseOffsetY + 250, 260, Font_NormalBlack);
	} else {
		Widget_GameText_drawCentered(44, 127,
			baseOffsetX + 15, baseOffsetY + 242, 260, Font_NormalBlack);
		int width;
		if (Data_Scenario.winCriteria.cultureEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.culture, '@', " ",
				baseOffsetX + 90, baseOffsetY + 270, Font_NormalBlack);
			Widget_GameText_draw(44, 129,
				baseOffsetX + 90 + width, baseOffsetY + 270, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria.prosperityEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.prosperity, '@', " ",
				baseOffsetX + 90, baseOffsetY + 286, Font_NormalBlack);
			Widget_GameText_draw(44, 130,
				baseOffsetX + 90 + width, baseOffsetY + 286, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria.peaceEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.peace, '@', " ",
				baseOffsetX + 90, baseOffsetY + 302, Font_NormalBlack);
			Widget_GameText_draw(44, 131,
				baseOffsetX + 90 + width, baseOffsetY + 302, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria.favorEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.favor, '@', " ",
				baseOffsetX + 90, baseOffsetY + 318, Font_NormalBlack);
			Widget_GameText_draw(44, 132,
				baseOffsetX + 90 + width, baseOffsetY + 318, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria_populationEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria_population, '@', " ",
				baseOffsetX + 90, baseOffsetY + 334, Font_NormalBlack);
			Widget_GameText_draw(44, 133,
				baseOffsetX + 90 + width, baseOffsetY + 334, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria.timeLimitYearsEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.timeLimitYears, '@', " ",
				baseOffsetX + 90, baseOffsetY + 350, Font_NormalBlack);
			Widget_GameText_draw(44, 134,
				baseOffsetX + 90 + width, baseOffsetY + 350, Font_NormalBlack);
		}
		if (Data_Scenario.winCriteria.survivalYearsEnabled) {
			width = Widget_Text_drawNumber(Data_Scenario.winCriteria.survivalYears, '@', " ",
				baseOffsetX + 90, baseOffsetY + 366, Font_NormalBlack);
			Widget_GameText_draw(44, 135,
				baseOffsetX + 90 + width, baseOffsetY + 366, Font_NormalBlack);
		}
	}
	Widget_GameText_draw(44, 136, baseOffsetX + 100, baseOffsetY + 426, Font_NormalBlack);
}

void UI_CCKSelection_drawForeground()
{
	Widget_Button_drawImageButtons(Data_Screen.offset640x480.x, Data_Screen.offset640x480.y, imageButtons, 3);
	drawScenarioList();
}

void UI_CCKSelection_handleMouse()
{
	if (Data_Mouse.scrollDown) {
		buttonScroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		buttonScroll(0, 3);
	}
	if (handleScrollbarClick()) {
		return;
	}
	if (Widget_Button_handleImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y, imageButtons, 3)) {
		return;
	}
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		customButtons, 15, &focusButtonId);
}

static int handleScrollbarClick()
{
	if (Data_FileList.numFiles <= 15) {
		return 0;
	}
	if (!Data_Mouse.left.isDown) {
		return 0;
	}
	int x = Data_Screen.offset640x480.x;
	int y = Data_Screen.offset640x480.y;
	if (Data_Mouse.x >= x + 280 && Data_Mouse.x <= x + 312 &&
		Data_Mouse.y >= y + 245 && Data_Mouse.y <= y + 434) {
		int yOffset = Data_Mouse.y - (y + 245);
		if (yOffset > 164) {
			yOffset = 164;
		}
		int pct = Calc_getPercentage(yOffset, 164);
		scrollPosition = Calc_adjustWithPercentage(Data_FileList.numFiles - 15, pct);
		UI_Window_requestRefresh();
		return 1;
	}
	return 0;
}

static void buttonSelectItem(int index, int param2)
{
	if (index >= Data_FileList.numFiles) {
		return;
	}
	selectedItem = scrollPosition + index;
	strcpy(Data_FileList.selectedScenario, Data_FileList.files[selectedItem]);
	GameFile_loadScenario(Data_FileList.selectedScenario);
	FileSystem_removeExtension(Data_FileList.selectedScenario);
	UI_Window_requestRefresh();
}

static void buttonScroll(int isDown, int numLines)
{
	if (Data_FileList.numFiles > 15) {
		if (isDown) {
			scrollPosition += numLines;
			if (scrollPosition > Data_FileList.numFiles - 15) {
				scrollPosition = Data_FileList.numFiles - 15;
			}
		} else {
			scrollPosition -= numLines;
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
	}
}

static void buttonStartScenario(int param1, int param2)
{
	// TODO
}
