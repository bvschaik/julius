#include "PopupDialog.h"
#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../CityInfo.h"
#include "../Data/CityInfo.h"
#include "../Data/Screen.h"
#include "../Data/Mouse.h"

#define GROUP 5

static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static ImageButton buttons[] = {
	{192, 100, 34, 34, ImageButton_Normal, 96, 0, buttonOk, Widget_Button_doNothing, 1, 0, 1},
	{256, 100, 34, 34, ImageButton_Normal, 96, 4, buttonCancel, Widget_Button_doNothing, 0, 0, 1},
};

static struct {
	int msgId;
	int okClicked;
	void (*closeFunc)(int accepted);
	int hasButtons;
	int x;
	int y;
} data;

void UI_PopupDialog_show(int msgId, void (*closeFunc)(int accepted), int hasOkCancelButtons)
{
	if (UI_Window_getId() == Window_PopupDialog) {
		// don't show popup over popup
		return;
	}
	data.msgId = msgId;
	data.okClicked = 0;
	data.closeFunc = closeFunc;
	data.hasButtons = hasOkCancelButtons;
	data.x = Data_Screen.offset640x480.x + 80;
	data.y = Data_Screen.offset640x480.y + 80;
	UI_Window_goTo(Window_PopupDialog);
}

void UI_PopupDialog_drawBackground()
{
	Widget_Panel_drawOuterPanel(data.x, data.y, 30, 10);
	Widget_GameText_drawCentered(GROUP, data.msgId, data.x, data.y + 20, 480, Font_LargeBlack);
	if (Widget_GameText_getWidth(GROUP, data.msgId + 1, Font_NormalBlack) >= 420) {
		Widget_GameText_drawMultiline(GROUP, data.msgId + 1, data.x + 30, data.y + 60, 420, Font_NormalBlack);
	} else {
		Widget_GameText_drawCentered(GROUP, data.msgId + 1, data.x, data.y + 60, 480, Font_NormalBlack);
	}
}

void UI_PopupDialog_drawForeground()
{
	if (data.hasButtons) {
		Widget_Button_drawImageButtons(data.x, data.y, buttons, 2);
	} else {
		Widget_GameText_drawCentered(13, 1, data.x, data.y + 128, 480, Font_NormalBlack);
	}
}

void UI_PopupDialog_handleMouse()
{
	if (data.hasButtons) {
		Widget_Button_handleImageButtons(data.x, data.y, buttons, 2, 0);
	} else if (Data_Mouse.right.wentUp) {
		data.closeFunc(0);
		UI_Window_goBack();
	}
}

void buttonOk(int param1, int param2)
{
	UI_Window_goBack();
	data.closeFunc(1);
}

void buttonCancel(int param1, int param2)
{
	UI_Window_goBack();
	data.closeFunc(0);
}
