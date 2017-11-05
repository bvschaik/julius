#include "allwindows.h"
#include "window.h"
#include "messagedialog.h"
#include "core/calc.h"
#include "graphics.h"
#include "playermessage.h"
#include "widget.h"

#include "data/constants.hpp"
#include "data/message.hpp"
#include "data/screen.hpp"

#include "core/lang.h"
#include "graphics/image.h"

static void buttonHelp(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonScroll(int param1, int param2);
static void buttonMessage(int param1, int param2);
static void buttonDelete(int param1, int param2);

static void handleMouseScrollbar(const mouse *m);

static ImageButton imageButtonHelp = {
	0, 0, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget_Button_doNothing, 0, 0, 1
};
static ImageButton imageButtonClose = {
	0, 0, 24, 24, ImageButton_Normal, 134, 4, buttonClose, Widget_Button_doNothing, 0, 0, 1
};
static ImageButton imageButtonScrollUp = {
	0, 0, 39, 26, ImageButton_Scroll, 96, 8, buttonScroll, Widget_Button_doNothing, 0, 1, 1
};
static ImageButton imageButtonScrollDown = {
	0, 0, 39, 26, ImageButton_Scroll, 96, 12, buttonScroll, Widget_Button_doNothing, 1, 1, 1
};
static CustomButton customButtonsMessages[] = {
	{0, 0, 412, 18, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 0, 0},
	{0, 20, 412, 38, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 1, 0},
	{0, 40, 412, 58, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 2, 0},
	{0, 60, 412, 78, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 3, 0},
	{0, 80, 412, 98, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 4, 0},
	{0, 100, 412, 118, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 5, 0},
	{0, 120, 412, 138, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 6, 0},
	{0, 140, 412, 158, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 7, 0},
	{0, 160, 412, 178, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 8, 0},
	{0, 180, 412, 198, CustomButton_OnMouseUp, buttonMessage, buttonDelete, 9, 0},
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
	Widget_GameText_drawCentered(63, 0, data.x, data.y + 16, 16 * data.widthBlocks, FONT_LARGE_BLACK);
	Widget_Panel_drawInnerPanel(data.xText, data.yText, data.textWidthBlocks, data.textHeightBlocks);

	if (Data_Message.totalMessages > 0) {
		Widget_GameText_draw(63, 2, data.xText + 42, data.yText - 12, FONT_SMALL_PLAIN);
		Widget_GameText_draw(63, 3, data.xText + 180, data.yText - 12, FONT_SMALL_PLAIN);
		Widget_GameText_drawMultiline(63, 4,
			data.xText + 50, data.yText + 12 + 16 * data.textHeightBlocks,
			16 * data.textWidthBlocks - 100, FONT_NORMAL_BLACK);
	} else {
		Widget_GameText_drawMultiline(63, 1,
			data.xText + 16, data.yText + 80,
			16 * data.textWidthBlocks - 48, FONT_NORMAL_GREEN);
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
		if (lang_get_message(messageId)->message_type == MESSAGE_TYPE_DISASTER) {
			graphicOffset = 2;
		}
		if (Data_Message.messages[index].readFlag) {
			Graphics_drawImage(image_group(ID_Graphic_MessageIcon) + 15 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		} else {
			Graphics_drawImage(image_group(ID_Graphic_MessageIcon) + 14 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		}
		font_t font = FONT_NORMAL_WHITE;
		if (focusButtonId == i + 1) {
			font = FONT_NORMAL_RED;
		}
		int width = Widget_GameText_draw(25, Data_Message.messages[index].month,
			data.xText + 42, data.yText + 8 + 20 * i, font);
		Widget_GameText_drawYear(Data_Message.messages[index].year,
			data.xText + 42 + width, data.yText + 8 + 20 * i, font);
		Widget_Text_draw(
			lang_get_message(messageId)->title.text,
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
			pctScrolled = calc_percentage(Data_Message.scrollPosition, Data_Message.maxScrollPosition);
		}
		int dotOffset = calc_adjust_with_percentage(16 * data.textHeightBlocks - 77, pctScrolled);
		if (Data_Message.isDraggingScrollbar) {
			dotOffset = Data_Message.scrollPositionDrag;
		}
		Graphics_drawImage(image_group(ID_Graphic_PanelButton) + 39,
			data.xText + 9 + 16 * data.textWidthBlocks, data.yText + 26 + dotOffset);
	}
}

void UI_PlayerMessageList_handleMouse(const mouse *m)
{
	if (m->scrolled == SCROLL_DOWN) {
		buttonScroll(1, 3);
	} else if (m->scrolled == SCROLL_UP) {
		buttonScroll(0, 3);
	}
	int buttonId;
	Widget_Button_handleImageButtons(data.x + 16, data.y + 16 * data.heightBlocks - 42, &imageButtonHelp, 1, &buttonId);
	if (buttonId) {
		focusButtonId = 11;
		return;
	}
	Widget_Button_handleImageButtons(data.x + 16 * data.widthBlocks - 38,
		data.y + 16 * data.heightBlocks - 36, &imageButtonClose, 1, &buttonId);
	if (buttonId) {
		focusButtonId = 12;
		return;
	}
	Widget_Button_handleImageButtons(data.xText + 16 * data.textWidthBlocks, data.yText, &imageButtonScrollUp, 1, &buttonId);
	if (buttonId) {
		focusButtonId = 13;
		return;
	}
	Widget_Button_handleImageButtons(data.xText + 16 * data.textWidthBlocks,
		data.yText + 16 * data.textHeightBlocks - 26, &imageButtonScrollDown, 1, &buttonId);
	if (buttonId) {
		focusButtonId = 13;
		return;
	}
	int oldFocusButtonId = focusButtonId;
	if (Widget_Button_handleCustomButtons(
		data.xText, data.yText + 4, customButtonsMessages, 10, &focusButtonId)) {
		if (oldFocusButtonId != focusButtonId) {
			UI_Window_requestRefresh();
		}
		return;
	}
	handleMouseScrollbar(m);
}

static void handleMouseScrollbar(const mouse *m)
{
	if (Data_Message.maxScrollPosition <= 0 || !m->left.went_down) {
		return;
	}
	int scrollbarX = data.xText + 16 * data.textWidthBlocks + 1;
	int scrollbarY = data.yText + 26;
	int scrollbarHeight = 16 * data.textHeightBlocks - 52;
	if (m->x >= scrollbarX && m->x <= scrollbarX + 40 &&
		m->y >= scrollbarY && m->y <= scrollbarY + scrollbarHeight) {
		int dotOffset = m->y - data.yText - 11;
		if (dotOffset > scrollbarHeight) {
			dotOffset = scrollbarHeight;
		}
		int pctScrolled = calc_percentage(dotOffset, scrollbarHeight);
		Data_Message.scrollPosition = calc_adjust_with_percentage(
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

void UI_PlayerMessageList_getTooltip(struct TooltipContext *c)
{
	if (focusButtonId == 11) {
		c->textId = 1;
	} else if (focusButtonId == 12) {
		c->textId = 2;
	} else {
		return;
	}
	c->type = TooltipType_Button;
}
