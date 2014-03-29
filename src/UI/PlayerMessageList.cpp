#include "AllWindows.h"
#include "Window.h"
#include "MessageDialog.h"
#include "../Calc.h"
#include "../Graphics.h"
#include "../PlayerMessage.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Language.h"
#include "../Data/Message.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

static void buttonHelp(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonScroll(int param1, int param2);
static void buttonMessage(int param1, int param2);
static void buttonDelete(int param1, int param2);

static void handleMouseScrollbar();

static ImageButton imageButtonHelp = {
	0, 0, 27, 27, 4, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0
};
static ImageButton imageButtonClose = {
	0, 0, 24, 24, 4, 134, 4, buttonClose, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0
};
static ImageButton imageButtonScrollUp = {
	0, 0, 39, 26, 6, 96, 8, buttonScroll, Widget_Button_doNothing, 1, 0, 0, 0, 0, 1
};
static ImageButton imageButtonScrollDown = {
	0, 0, 39, 26, 6, 96, 12, buttonScroll, Widget_Button_doNothing, 1, 0, 0, 0, 1, 1
};
static CustomButton customButtonsMessages[] = {
	{0, 0, 412, 18, buttonMessage, buttonDelete, 3, 0, 0},
	{0, 20, 412, 38, buttonMessage, buttonDelete, 3, 1, 0},
	{0, 40, 412, 58, buttonMessage, buttonDelete, 3, 2, 0},
	{0, 60, 412, 78, buttonMessage, buttonDelete, 3, 3, 0},
	{0, 80, 412, 98, buttonMessage, buttonDelete, 3, 4, 0},
	{0, 100, 412, 118, buttonMessage, buttonDelete, 3, 5, 0},
	{0, 120, 412, 138, buttonMessage, buttonDelete, 3, 6, 0},
	{0, 140, 412, 158, buttonMessage, buttonDelete, 3, 7, 0},
	{0, 160, 412, 178, buttonMessage, buttonDelete, 3, 8, 0},
	{0, 180, 412, 198, buttonMessage, buttonDelete, 3, 9, 0},
};

static struct {
	int x;
	int y;
	int widthBlocks;
	int heightBlocks;
	int xText;
	int yText;
	int textWidthBlocks;
	int textHeightBlocks;
} data;

static int focusButtonId;

void UI_PlayerMessageList_init()
{
	PlayerMessage_sortMessages();
}

void UI_PlayerMessageList_drawBackground()
{
	UI_City_drawBackground();
	UI_City_drawForeground();

	data.x = Data_Screen.offset640x480.x;
	data.y = Data_Screen.offset640x480.y;
	data.widthBlocks = 30;
	data.heightBlocks = 22;
	data.xText = data.x + 16;
	data.yText = data.y + 80;
	data.textWidthBlocks = data.widthBlocks - 4;
	data.textHeightBlocks = data.heightBlocks - 9;

	Widget_Panel_drawOuterPanel(data.x, data.y, data.widthBlocks, data.heightBlocks);
	Widget_GameText_drawCentered(63, 0, data.x, data.y + 16, 16 * data.widthBlocks, Font_LargeBlack);
	Widget_Panel_drawInnerPanel(data.xText, data.yText, data.textWidthBlocks, data.textHeightBlocks);

	if (Data_Message.totalMessages > 0) {
		Widget_GameText_draw(63, 2, data.xText + 42, data.yText - 12, Font_SmallPlain);
		Widget_GameText_draw(63, 3, data.xText + 180, data.yText - 12, Font_SmallPlain);
		Widget_GameText_drawMultiline(63, 4,
			data.xText + 50, data.yText + 12 + 16 * data.textHeightBlocks,
			16 * data.textWidthBlocks - 100, Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(63, 1,
			data.xText + 16, data.yText + 80,
			16 * data.textWidthBlocks - 48, Font_NormalGreen);
	}
}

void UI_PlayerMessageList_drawForeground()
{
	Widget_Button_drawImageButtons(
		data.x + 16, data.y + 16 * data.heightBlocks - 42,
		&imageButtonHelp, 1);
	Widget_Button_drawImageButtons(
		data.x + 16 * data.widthBlocks - 38, data.y + 16 * data.heightBlocks - 36,
		&imageButtonClose, 1);

	if (Data_Message.totalMessages <= 0) {
		return;
	}

	int max = Data_Message.totalMessages < 10 ? Data_Message.totalMessages : 10;
	int index = Data_Message.scrollPosition;
	for (int i = 0; i < max; i++, index++) {
		int messageId = PlayerMessage_getMessageTextId(Data_Message.messages[index].messageType);
		int graphicOffset = 0;
		if (Data_Language_Message.index[messageId].messageType == MessageType_Disaster) {
			graphicOffset = 2;
		}
		if (Data_Message.messages[index].readFlag) {
			Graphics_drawImage(GraphicId(ID_Graphic_MessageIcon) + 15 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		} else {
			Graphics_drawImage(GraphicId(ID_Graphic_MessageIcon) + 14 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		}
		Font font = Font_NormalWhite;
		if (focusButtonId == i + 1) {
			font = Font_NormalRed;
		}
		int width = Widget_GameText_draw(25, Data_Message.messages[index].month,
			data.xText + 42, data.yText + 8 + 20 * i, font);
		Widget_GameText_drawYear(Data_Message.messages[index].year,
			data.xText + 42 + width, data.yText + 8 + 20 * i, font);
		Widget_Text_draw(
			&Data_Language_Message.data[Data_Language_Message.index[messageId].titleOffset],
			data.xText + 180, data.yText + 8 + 20 * i, font, 0);
	}
	if (Data_Message.maxScrollPosition > 0) {
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks, data.yText,
			&imageButtonScrollUp, 1);
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks, data.yText + 16 * data.textHeightBlocks - 26,
			&imageButtonScrollDown, 1);
		int pctScrolled;
		if (Data_Message.scrollPosition <= 0) {
			pctScrolled = 0;
		} else if (Data_Message.scrollPosition >= Data_Message.maxScrollPosition) {
			pctScrolled = 100;
		} else {
			pctScrolled = Calc_getPercentage(Data_Message.scrollPosition, Data_Message.maxScrollPosition);
		}
		int dotOffset = Calc_adjustWithPercentage(16 * data.textHeightBlocks - 77, pctScrolled);
		if (Data_Message.isDraggingScrollbar) {
			dotOffset = Data_Message.scrollPositionDrag;
		}
		Graphics_drawImage(GraphicId(ID_Graphic_PanelButton) + 39,
			data.xText + 9 + 16 * data.textWidthBlocks, data.yText + 26 + dotOffset);
	}
}

#include <cstdio>

void UI_PlayerMessageList_handleMouse()
{
	if (Data_Mouse.scrollDown) {
		buttonScroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		buttonScroll(0, 3);
	}
	if (Widget_Button_handleImageButtons(
		data.x + 16, data.y + 16 * data.heightBlocks - 42,
		&imageButtonHelp, 1)) {
			focusButtonId = 11;
			return;
	}
	if (Widget_Button_handleImageButtons(
		data.x + 16 * data.widthBlocks - 38, data.y + 16 * data.heightBlocks - 36,
		&imageButtonClose, 1)) {
			focusButtonId = 12;
			return;
	}
	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks, data.yText,
		&imageButtonScrollUp, 1)) {
			focusButtonId = 13;
			return;
	}
	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks, data.yText + 16 * data.textHeightBlocks - 26,
		&imageButtonScrollDown, 1)) {
			focusButtonId = 13;
			return;
	}
	if (Widget_Button_handleCustomButtons(
		data.xText, data.yText + 4, customButtonsMessages, 10, &focusButtonId)) {
			return;
	}
	handleMouseScrollbar();
}

static void handleMouseScrollbar()
{
	if (Data_Message.maxScrollPosition <= 0 || !Data_Mouse.leftDown) {
		return;
	}
	int scrollbarX = data.xText + 16 * data.textWidthBlocks + 1;
	int scrollbarY = data.yText + 26;
	int scrollbarHeight = 16 * data.textHeightBlocks - 52;
	if (Data_Mouse.x >= scrollbarX && Data_Mouse.x <= scrollbarX + 40 &&
		Data_Mouse.y >= scrollbarY && Data_Mouse.y <= scrollbarY + scrollbarHeight) {
		int dotOffset = Data_Mouse.y - data.yText - 11;
		if (dotOffset > scrollbarHeight) {
			dotOffset = scrollbarHeight;
		}
		int pctScrolled = Calc_getPercentage(dotOffset, scrollbarHeight);
		Data_Message.scrollPosition = Calc_adjustWithPercentage(
			Data_Message.maxScrollPosition, pctScrolled);
		Data_Message.isDraggingScrollbar = 1;
		Data_Message.scrollPositionDrag = dotOffset - 25;
		if (Data_Message.scrollPositionDrag < 0) {
			Data_Message.scrollPositionDrag = 0;
		}
		UI_Window_requestRefresh();
	}
}

static void buttonScroll(int isDown, int numLines)
{
	if (isDown) {
		Data_Message.scrollPosition += numLines;
		if (Data_Message.scrollPosition > Data_Message.maxScrollPosition) {
			Data_Message.scrollPosition = Data_Message.maxScrollPosition;
		}
	} else {
		Data_Message.scrollPosition -= numLines;
		if (Data_Message.scrollPosition < 0) {
			Data_Message.scrollPosition = 0;
		}
	}
	Data_Message.isDraggingScrollbar = 0;
	UI_Window_requestRefresh();
}

static void buttonHelp(int param1, int param2)
{
	UI_MessageDialog_show(MessageDialog_Messages, 0);
}

static void buttonClose(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void buttonMessage(int param1, int param2)
{
	int id = Data_Message.currentMessageId = Data_Message.scrollPosition + param1;
	if (id < Data_Message.totalMessages) {
		Data_Message.messages[id].readFlag = 1;
		int type = Data_Message.messages[id].messageType;
		UI_MessageDialog_setPlayerMessage(
			Data_Message.messages[id].year,
			Data_Message.messages[id].month,
			Data_Message.messages[id].param1,
			Data_Message.messages[id].param2,
			PlayerMessage_getAdvisorForMessageType(type),
			0);
		UI_MessageDialog_show(PlayerMessage_getMessageTextId(type), 0);
	}
}

static void buttonDelete(int param1, int param2)
{
	int id = Data_Message.currentMessageId = Data_Message.scrollPosition + param1;
	if (id < Data_Message.totalMessages) {
		Data_Message.messages[id].messageType = 0;
		PlayerMessage_sortMessages();
		if (Data_Message.scrollPosition >= Data_Message.maxScrollPosition) {
			--Data_Message.scrollPosition;
		}
		if (Data_Message.scrollPosition < 0) {
			Data_Message.scrollPosition = 0;
		}
		UI_Window_requestRefresh();
	}
}
