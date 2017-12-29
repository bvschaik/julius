#include "AllWindows.h"
#include "MessageDialog.h"
#include "core/calc.h"
#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/Screen.h"

#include "city/message.h"
#include "core/lang.h"

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

    int scrollPosition;
    int maxScrollPosition;
    int isDraggingScrollbar;
    int scrollPositionDrag;
} data;

static int focusButtonId;

static void update_scroll_position()
{
    int totalMessages = city_message_count();
    if (totalMessages <= 10) {
        data.scrollPosition = 0;
        data.maxScrollPosition = 0;
    } else {
        data.maxScrollPosition = totalMessages - 10;
        if (data.scrollPosition >= data.maxScrollPosition) {
            data.scrollPosition = data.maxScrollPosition;
        }
    }
}

void UI_PlayerMessageList_resetScroll()
{
    data.scrollPosition = 0;
    data.maxScrollPosition = 0;
}

void UI_PlayerMessageList_init()
{
    city_message_sort_and_compact();
    update_scroll_position();
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

	if (city_message_count() > 0) {
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

    int totalMessages = city_message_count();
	if (totalMessages <= 0) {
		return;
	}

	int max = totalMessages < 10 ? totalMessages : 10;
	int index = data.scrollPosition;
	for (int i = 0; i < max; i++, index++) {
        const city_message *msg = city_message_get(index);
		int messageId = city_message_get_text_id(msg->message_type);
		int graphicOffset = 0;
		if (lang_get_message(messageId)->message_type == MESSAGE_TYPE_DISASTER) {
			graphicOffset = 2;
		}
		if (msg->is_read) {
			Graphics_drawImage(image_group(GROUP_MESSAGE_ICON) + 15 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		} else {
			Graphics_drawImage(image_group(GROUP_MESSAGE_ICON) + 14 + graphicOffset,
				data.xText + 12, data.yText + 6 + 20 * i);
		}
		font_t font = FONT_NORMAL_WHITE;
		if (focusButtonId == i + 1) {
			font = FONT_NORMAL_RED;
		}
		int width = Widget_GameText_draw(25, msg->month,
			data.xText + 42, data.yText + 8 + 20 * i, font);
		Widget_GameText_drawYear(msg->year,
			data.xText + 42 + width, data.yText + 8 + 20 * i, font);
		Widget_Text_draw(
			lang_get_message(messageId)->title.text,
			data.xText + 180, data.yText + 8 + 20 * i, font, 0);
	}
	if (data.maxScrollPosition > 0) {
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks, data.yText,
			&imageButtonScrollUp, 1);
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks, data.yText + 16 * data.textHeightBlocks - 26,
			&imageButtonScrollDown, 1);
		int pctScrolled;
		if (data.scrollPosition <= 0) {
			pctScrolled = 0;
		} else if (data.scrollPosition >= data.maxScrollPosition) {
			pctScrolled = 100;
		} else {
			pctScrolled = calc_percentage(data.scrollPosition, data.maxScrollPosition);
		}
		int dotOffset = calc_adjust_with_percentage(16 * data.textHeightBlocks - 77, pctScrolled);
		if (data.isDraggingScrollbar) {
			dotOffset = data.scrollPositionDrag;
		}
		Graphics_drawImage(image_group(GROUP_PANEL_BUTTON) + 39,
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
	if (data.maxScrollPosition <= 0 || !m->left.went_down) {
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
		data.scrollPosition = calc_adjust_with_percentage(data.maxScrollPosition, pctScrolled);
		data.isDraggingScrollbar = 1;
		data.scrollPositionDrag = dotOffset - 25;
		if (data.scrollPositionDrag < 0) {
			data.scrollPositionDrag = 0;
		}
		UI_Window_requestRefresh();
	}
}

static void buttonScroll(int isDown, int numLines)
{
	if (isDown) {
		data.scrollPosition += numLines;
		if (data.scrollPosition > data.maxScrollPosition) {
			data.scrollPosition = data.maxScrollPosition;
		}
	} else {
		data.scrollPosition -= numLines;
		if (data.scrollPosition < 0) {
			data.scrollPosition = 0;
		}
	}
	data.isDraggingScrollbar = 0;
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
	int id = city_message_set_current(data.scrollPosition + param1);
	if (id < city_message_count()) {
        const city_message *msg = city_message_get(id);
		city_message_mark_read(id);
		UI_MessageDialog_setPlayerMessage(
			msg->year, msg->month, msg->param1, msg->param2,
			city_message_get_advisor(msg->message_type),
			0);
		UI_MessageDialog_show(city_message_get_text_id(msg->message_type), 0);
	}
}

static void buttonDelete(int param1, int param2)
{
	int id = city_message_set_current(data.scrollPosition + param1);
	if (id < city_message_count()) {
		city_message_delete(id);
		update_scroll_position();
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
