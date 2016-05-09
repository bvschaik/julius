#include "FileDialog.h"
#include "Window.h"

#include "../Calc.h"
#include "../FileSystem.h"
#include "../GameFile.h"
#include "../Graphics.h"
#include "../KeyboardInput.h"
#include "../Time.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/FileList.h"
#include "../Data/KeyboardInput.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

#include <string.h>

static void drawScrollbarDot();
static int handleScrollbarClick();
static void buttonOkCancel(int isOk, int param2);
static void buttonScroll(int isDown, int numLines);
static void buttonSelectItem(int index, int numLines);

static ImageButton imageButtons[] = {
	{344, 335, 34, 34, ImageButton_Normal, 96, 0, buttonOkCancel, Widget_Button_doNothing, 1, 0, 1},
	{392, 335, 34, 34, ImageButton_Normal, 96, 4, buttonOkCancel, Widget_Button_doNothing, 0, 0, 1},
	{464, 120, 34, 34, ImageButton_Scroll, 96, 8, buttonScroll, Widget_Button_doNothing, 0, 1, 1},
	{464, 300, 34, 34, ImageButton_Scroll, 96, 12, buttonScroll, Widget_Button_doNothing, 1, 1, 1},
};
static CustomButton customButtons[] = {
	{160, 128, 448, 144, buttonSelectItem, Widget_Button_doNothing, 1, 0, 0},
	{160, 144, 448, 160, buttonSelectItem, Widget_Button_doNothing, 1, 1, 0},
	{160, 160, 448, 176, buttonSelectItem, Widget_Button_doNothing, 1, 2, 0},
	{160, 176, 448, 192, buttonSelectItem, Widget_Button_doNothing, 1, 3, 0},
	{160, 192, 448, 208, buttonSelectItem, Widget_Button_doNothing, 1, 4, 0},
	{160, 208, 448, 224, buttonSelectItem, Widget_Button_doNothing, 1, 5, 0},
	{160, 224, 448, 240, buttonSelectItem, Widget_Button_doNothing, 1, 6, 0},
	{160, 240, 448, 256, buttonSelectItem, Widget_Button_doNothing, 1, 7, 0},
	{160, 256, 448, 272, buttonSelectItem, Widget_Button_doNothing, 1, 8, 0},
	{160, 272, 448, 288, buttonSelectItem, Widget_Button_doNothing, 1, 9, 0},
	{160, 288, 448, 304, buttonSelectItem, Widget_Button_doNothing, 1, 10, 0},
	{160, 304, 448, 320, buttonSelectItem, Widget_Button_doNothing, 1, 11, 0},
};

#define NOT_EXIST_MESSAGE_TIMEOUT 200

static TimeMillis messageNotExistTimeUntil;
static int dialogType;
static int focusButtonId;
static int scrollPosition;

void UI_FileDialog_show(int type)
{
	dialogType = type;
	messageNotExistTimeUntil = 0;
	scrollPosition = 0;

	FileSystem_findFilesWithExtension("sav");

	strcpy(Data_FileList.selectedCity, Data_FileList.lastLoadedCity);
	Data_KeyboardInput.accepted = 0;
	KeyboardInput_initInput(2);
	KeyboardInput_home();
	KeyboardInput_end();

	UI_Window_goTo(Window_FileDialog);
}

void UI_FileDialog_drawBackground()
{
	// do nothing
}

void UI_FileDialog_drawForeground()
{
	char file[100];

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(baseOffsetX + 128, baseOffsetY + 40, 24, 21);
	Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 80, 20, 2);
	Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 120, 20, 13);

	// title
	if (messageNotExistTimeUntil && Time_getMillis() < messageNotExistTimeUntil) {
		Widget_GameText_drawCentered(43, 2,
			baseOffsetX + 160, baseOffsetY + 50, 304, Font_LargeBlack);
	} else if (dialogType == FileDialogType_Delete) {
		Widget_GameText_drawCentered(43, 6,
			baseOffsetX + 160, baseOffsetY + 50, 304, Font_LargeBlack);
	} else {
		Widget_GameText_drawCentered(43, dialogType,
			baseOffsetX + 160, baseOffsetY + 50, 304, Font_LargeBlack);
	}
	Widget_GameText_draw(43, 5, baseOffsetX + 224, baseOffsetY + 342, Font_NormalBlack);

	for (int i = 0; i < 12; i++) {
		Font font = Font_NormalGreen;
		if (focusButtonId == i + 1) {
			font = Font_NormalWhite;
		}
		strcpy(file, Data_FileList.files[scrollPosition + i]);
		FileSystem_removeExtension(file);
		Widget_Text_draw(file, baseOffsetX + 160, baseOffsetY + 130 + 16 * i, font, 0);
	}

	Widget_Button_drawImageButtons(baseOffsetX, baseOffsetY, imageButtons, 4);
	Widget_Text_captureCursor();
	Widget_Text_draw(Data_FileList.selectedCity, baseOffsetX + 160, baseOffsetY + 90, Font_NormalWhite, 0);
	Widget_Text_drawCursor(baseOffsetX + 160, baseOffsetY + 91);
	drawScrollbarDot();
}

static void drawScrollbarDot()
{
	if (Data_FileList.numFiles > 12) {
		int pct;
		if (scrollPosition <= 0) {
			pct = 0;
		} else if (scrollPosition + 12 >= Data_FileList.numFiles) {
			pct = 100;
		} else {
			pct = Calc_getPercentage(scrollPosition, Data_FileList.numFiles - 12);
		}
		int yOffset = Calc_adjustWithPercentage(130, pct);
		Graphics_drawImage(GraphicId(ID_Graphic_PanelButton) + 39,
			Data_Screen.offset640x480.x + 472,
			Data_Screen.offset640x480.y + 145 + yOffset);
	}
}

void UI_FileDialog_handleMouse()
{
	if (Data_Mouse.scrollDown) {
		buttonScroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		buttonScroll(0, 3);
	}

	KeyboardInput_initInput(2);
	if (Data_KeyboardInput.accepted) {
		Data_KeyboardInput.accepted = 0;
		buttonOkCancel(1, 0);
		return;
	}

	if (Data_Mouse.right.wentDown) {
		UI_Window_goBack();
		return;
	}
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	if (!Widget_Button_handleCustomButtons(xOffset, yOffset, customButtons, 12, &focusButtonId)) {
		if (!Widget_Button_handleImageButtons(xOffset, yOffset, imageButtons, 4, 0)) {
			handleScrollbarClick();
		}
	}
}

static int handleScrollbarClick()
{
	if (Data_FileList.numFiles <= 12) {
		return 0;
	}
	if (!Data_Mouse.left.isDown) {
		return 0;
	}
	int x = Data_Screen.offset640x480.x;
	int y = Data_Screen.offset640x480.y;
	if (Data_Mouse.x >= x + 464 && Data_Mouse.x <= x + 496 &&
		Data_Mouse.y >= y + 145 && Data_Mouse.y <= y + 300) {
		int yOffset = Data_Mouse.y - (y + 145);
		if (yOffset > 130) {
			yOffset = 130;
		}
		int pct = Calc_getPercentage(yOffset, 130);
		scrollPosition = Calc_adjustWithPercentage(Data_FileList.numFiles - 12, pct);
		UI_Window_requestRefresh();
		return 1;
	}
	return 0;
}

static void buttonOkCancel(int isOk, int param2)
{
	if (!isOk) {
		UI_Window_goBack();
		return;
	}

	FileSystem_removeExtension(Data_FileList.selectedCity);
	FileSystem_appendExtension(Data_FileList.selectedCity, "sav");

	if (dialogType != FileDialogType_Save && !FileSystem_fileExists(Data_FileList.selectedCity)) {
		FileSystem_removeExtension(Data_FileList.selectedCity);
		messageNotExistTimeUntil = Time_getMillis() + NOT_EXIST_MESSAGE_TIMEOUT;
		return;
	}
	if (dialogType == FileDialogType_Load) {
		GameFile_loadSavedGame(Data_FileList.selectedCity);
		UI_Window_goTo(Window_City);
	} else if (dialogType == FileDialogType_Save) {
		GameFile_writeSavedGame(Data_FileList.selectedCity);
		UI_Window_goTo(Window_City);
	} else if (dialogType == FileDialogType_Delete) {
		if (GameFile_deleteSavedGame(Data_FileList.selectedCity)) {
			FileSystem_findFilesWithExtension("sav");
			if (scrollPosition + 12 >= Data_FileList.numFiles) {
				--scrollPosition;
			}
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
	}
	
	FileSystem_removeExtension(Data_FileList.selectedCity);
	strcpy(Data_FileList.lastLoadedCity, Data_FileList.selectedCity);
}

static void buttonScroll(int isDown, int numLines)
{
	if (Data_FileList.numFiles > 12) {
		if (isDown) {
			scrollPosition += numLines;
			if (scrollPosition > Data_FileList.numFiles - 12) {
				scrollPosition = Data_FileList.numFiles - 12;
			}
		} else {
			scrollPosition -= numLines;
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
		messageNotExistTimeUntil = 0;
	}
}

static void buttonSelectItem(int index, int param2)
{
	if (index < Data_FileList.numFiles) {
		memset(Data_FileList.selectedCity, 0, FILENAME_LENGTH);
		strcpy(Data_FileList.selectedCity, Data_FileList.files[scrollPosition + index]);
		FileSystem_removeExtension(Data_FileList.selectedCity);
		KeyboardInput_initInput(2);
		KeyboardInput_home();
		KeyboardInput_end();
		messageNotExistTimeUntil = 0;
	}
}
