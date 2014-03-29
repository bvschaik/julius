#include "MessageDialog.h"
#include "Window.h"
#include "AllWindows.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../Calc.h"
#include "../Resource.h"

#include "../Data/Buttons.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Empire.h"
#include "../Data/Graphics.h"
#include "../Data/Message.h"
#include "../Data/Mouse.h"
#include "../Data/Language.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

#define MAX_HISTORY 200

#define TEXT(offset) &Data_Language_Message.data[offset]

static void drawDialogNormal();
static void drawDialogVideo();
static void drawPlayerMessageContent(struct Data_Language_MessageEntry *msg);
static void drawScrollbar();
static void buttonScroll(int param1, int param2);
static void buttonBack(int param1, int param2);
static void buttonClose(int param1, int param2);

static ImageButton imageButtonBack = {
	0, 0, 31, 20, 4, 90, 8, buttonBack, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0
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

static struct {
	struct {
		int messageId;
		int scrollPosition;
	} history[200];
	int numHistory;

	int messageId;
	int scrollPosition;
	int maxScrollPosition;
	int numberOfLines;
	int dword_7e314c;
	int backgroundIsProvided;
	int showVideo;

	int x;
	int y;
	int xText;
	int yText;
	int textHeightBlocks;
	int textHeightLines;
	int textWidthBlocks;

	int isDraggingScroll;
	int scrollPositionDrag;
} data;

static struct {
	int year;
	int month;
	int param1;
	int param2;
	int messageAdvisor;
	int usePopup;
} playerMessage;

void UI_MessageDialog_setPlayerMessage(int year, int month,
									   int param1, int param2,
									   int messageAdvisor, int usePopup)
{
	playerMessage.year = year;
	playerMessage.month = month;
	playerMessage.param1 = param1;
	playerMessage.param2 = param2;
	playerMessage.messageAdvisor = messageAdvisor;
	playerMessage.usePopup = usePopup;
}

void UI_MessageDialog_show(int messageId, int backgroundIsProvided)
{
	for (int i = 0; i < MAX_HISTORY; i++) {
		data.history[i].messageId = 0;
		data.history[i].scrollPosition = 0;
	}
	data.numHistory = 0;
	data.scrollPosition = 0;
	data.numberOfLines = 0;
	data.dword_7e314c = 0;
	data.messageId = messageId;
	data.backgroundIsProvided = backgroundIsProvided;
	if (Data_Language_Message.index[messageId].videoLinkOffset) {
		data.showVideo = 1;
	} else {
		data.showVideo = 0;
	}
	if (playerMessage.usePopup != 1) {
		data.showVideo = 0;
	}
	Widget_RichText_clearLinks();
	UI_Window_goTo(Window_MessageDialog);
}

void UI_MessageDialog_drawBackground()
{
	if (data.showVideo) {
		drawDialogVideo();
	} else {
		drawDialogNormal();
	}
}

static void drawDialogNormal()
{
	Widget_RichText_setFonts(Font_NormalWhite, Font_NormalRed);
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.messageId];
	data.x = Data_Screen.offset640x480.x + msg->x;
	data.y = Data_Screen.offset640x480.y + msg->y;
	if (!data.backgroundIsProvided) {
		UI_City_drawBackground();
		UI_City_drawForeground();
	}
	int someOffset;
	if (msg->type == Type_Manual) {
		someOffset = 48;
	} else {
		someOffset = 32;
	}
	data.xText = data.x + 16;
	Widget_Panel_drawOuterPanel(data.x, data.y, msg->widthBlocks, msg->heightBlocks);
	// title
	if (msg->titleX) {
		Widget_Text_draw(TEXT(msg->titleOffset),
			data.x + msg->titleX, data.y + msg->titleY, Font_LargeBlack, 0);
		data.yText = data.y + 32;
	} else {
		if (msg->messageType == MessageType_Tutorial) {
			Widget_Text_drawCentered(TEXT(msg->titleOffset),
				data.x, data.y + msg->titleY, 16 * msg->widthBlocks, Font_LargeBlack, 0);
		} else {
			Widget_Text_drawCentered(TEXT(msg->titleOffset),
				data.x, data.y + 14, 16 * msg->widthBlocks, Font_LargeBlack, 0);
		}
		data.yText = data.y + 48;
	}
	// pictures
	if (msg->picture1_graphicId) {
		int graphicId;
		if (data.messageId) {
			graphicId = GraphicId(ID_Graphic_MessageImages) + msg->picture1_graphicId - 1;
		} else { // message id = 0 ==> about
			msg->picture1_x = 16;
			msg->picture1_y = 16;
			graphicId = GraphicId(ID_Graphic_BigPeople);
		}
		Graphics_drawImage(graphicId, data.x + msg->picture1_x, data.y + msg->picture1_y);
		if (data.y + msg->picture1_y + GraphicHeight(graphicId) + 8 > data.yText) {
			data.yText = data.y + msg->picture1_y + GraphicHeight(graphicId) + 8;
		}
	}
	if (msg->picture2_graphicId) {
		int graphicId = GraphicId(ID_Graphic_MessageImages) + msg->picture2_graphicId - 1;
		Graphics_drawImage(graphicId, data.x + msg->picture2_x, data.y + msg->picture2_y);
		if (data.y + msg->picture2_y + GraphicHeight(graphicId) + 8 > data.yText) {
			data.yText = data.y + msg->picture2_y + GraphicHeight(graphicId) + 8;
		}
	}
	// subtitle
	if (msg->subtitleX) {
		int width = 16 * msg->widthBlocks - 16 - msg->subtitleX;
		int height = Widget_Text_drawMultiline(TEXT(msg->subtitleOffset),
			data.x + msg->subtitleX, data.y + msg->subtitleY, width,Font_NormalBlack);
		if (data.y + msg->subtitleY + height > data.yText) {
			data.yText = data.y + msg->subtitleY + height;
		}
	}
	// determine number of lines
	if (!data.numberOfLines) {
		data.textHeightBlocks = msg->heightBlocks - 1 - (someOffset + data.yText - data.y) / 16;
		data.textHeightLines = data.textHeightBlocks - 1;
		data.textWidthBlocks = msg->widthBlocks - 4;
		
		imageButtonScrollUp.enabled = 1;
		imageButtonScrollDown.enabled = 1;

		data.numberOfLines = Widget_RichText_draw(TEXT(msg->contentOffset),
			data.xText + 8, data.yText + 6,
			16 * data.textWidthBlocks - 16, data.textHeightLines, 0, 1);
		if (data.numberOfLines <= data.textHeightLines) {
			data.textWidthBlocks = msg->widthBlocks - 2;
			data.maxScrollPosition = 0;
			imageButtonScrollUp.enabled = 0;
			imageButtonScrollDown.enabled = 0;
		} else {
			data.maxScrollPosition = data.numberOfLines - data.textHeightLines;
		}
		UI_Window_requestRefresh();
	}

	// content!
	Widget_Panel_drawInnerPanel(data.xText, data.yText, data.textWidthBlocks, data.textHeightBlocks);
	Graphics_setClipRectangle(data.xText + 3, data.yText + 3,
		16 * data.textWidthBlocks - 3, 16 * data.textHeightBlocks - 3);
	Widget_RichText_clearLinks();

	if (msg->type == Type_Message) {
		drawPlayerMessageContent(msg);
	} else {
		Widget_RichText_draw(TEXT(msg->contentOffset),
			data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
			data.textHeightLines, data.scrollPosition, 0);
	}
	Graphics_resetClipRectangle();
	drawScrollbar();
}

static void drawPlayerMessageContent(struct Data_Language_MessageEntry *msg)
{
	if (msg->messageType != MessageType_Tutorial) {
		int width = Widget_GameText_draw(25, playerMessage.month,
			data.xText + 10, data.yText + 6, Font_NormalWhite);
		width += Widget_GameText_drawYear(playerMessage.year,
			data.xText + 12 + width, data.yText + 6, Font_NormalWhite);
		if (msg->messageType == MessageType_Disaster && playerMessage.param1) {
			if (data.messageId == MessageDialog_Theft) {
				// param1 = denarii
				Widget_GameText_drawNumberWithDescription(8, 0, playerMessage.param1,
					data.x + 240, data.yText + 6, Font_NormalWhite);
			} else {
				// param1 = building type
				Widget_GameText_draw(41, playerMessage.param1,
					data.x + 240, data.yText + 6, Font_NormalWhite);
			}
		} else {
			width += Widget_GameText_draw(63, 5,
				data.xText + width + 80, data.yText + 6, Font_NormalWhite);
			Widget_Text_draw(Data_Settings.playerName,
				data.xText + width + 80, data.yText + 6, Font_NormalWhite, 0);
		}
	}
	int graphicId;
	int lines = 0;
	switch (msg->messageType) {
		case MessageType_Disaster:
		case MessageType_Invasion:
			Widget_GameText_draw(12, 1, data.x + 100, data.yText + 44, Font_NormalWhite);
			Widget_RichText_draw(TEXT(msg->contentOffset), data.xText + 8, data.yText + 86,
				16 * data.textWidthBlocks, data.textHeightLines, data.scrollPosition, 0);
			break;

		case MessageType_Emigration:
			if (Data_CityInfo.populationEmigrationCause >= 1 && Data_CityInfo.populationEmigrationCause <= 5) {
				Widget_GameText_draw(12, Data_CityInfo.populationEmigrationCause + 2,
					data.x + 64, data.yText + 44, Font_NormalWhite);
			}
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightLines, data.scrollPosition, 0);
			break;

		case MessageType_Tutorial:
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
				data.textHeightLines, data.scrollPosition, 0);
			break;

		case MessageType_TradeChange:
			graphicId = GraphicId(ID_Graphic_ResourceIcons) + playerMessage.param2;
			graphicId += Resource_getGraphicIdOffset(playerMessage.param2, 3);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_GameText_draw(21, Data_Empire_Cities[playerMessage.param1].cityNameId,
				data.x + 100, data.yText + 44, Font_NormalWhite);
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightLines, data.scrollPosition, 0);
			break;

		case MessageType_PriceChange:
			graphicId = GraphicId(ID_Graphic_ResourceIcons) + playerMessage.param2;
			graphicId += Resource_getGraphicIdOffset(playerMessage.param2, 3);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_Text_drawNumber(playerMessage.param1, '@', " Dn",
				data.x + 100, data.yText + 44, Font_NormalWhite);
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightLines, data.scrollPosition, 0);
			break;

		default:
			lines = Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 56, 16 * data.textWidthBlocks - 16,
				data.textHeightLines, data.scrollPosition, 0);
	}
	if (msg->messageType == MessageType_Imperial) {
		int yOffset = data.yText + 86 + lines * 16;
		Widget_Text_drawNumber(Data_Scenario.requests.amount[playerMessage.param1],
			'@', " ", data.xText + 8, yOffset, Font_NormalWhite);
		graphicId = GraphicId(ID_Graphic_ResourceIcons) +
			Data_Scenario.requests.resourceId[playerMessage.param1];
		graphicId += Resource_getGraphicIdOffset(
			Data_Scenario.requests.resourceId[playerMessage.param1], 3);
		Graphics_drawImage(graphicId, data.xText + 70, yOffset - 5);
		Widget_GameText_draw(23, Data_Scenario.requests.resourceId[playerMessage.param1],
			data.xText + 100, yOffset, Font_NormalWhite);
		if (Data_Scenario.requests_state[playerMessage.param1] <= 1) {
			int width = Widget_GameText_drawNumberWithDescription(8, 4,
				Data_Scenario.requests_monthsToComply[playerMessage.param1],
				data.xText + 200, yOffset, Font_NormalWhite);
			Widget_GameText_draw(12, 2, data.xText + 200 + width, yOffset, Font_NormalWhite);
		}
	}
}

static void drawDialogVideo()
{
	// TODO
	drawDialogNormal();
}

static void drawScrollbar()
{
	if (data.maxScrollPosition) {
		int pct;
		if (data.scrollPosition <= 0) {
			pct = 0;
		} else if (data.scrollPosition >= data.maxScrollPosition) {
			pct = 100;
		} else {
			pct = Calc_getPercentage(data.scrollPosition, data.maxScrollPosition);
		}
		int offset = Calc_adjustWithPercentage(16 * data.textHeightBlocks - 77, pct);
		if (data.isDraggingScroll) {
			offset = data.scrollPositionDrag;
		}
		Graphics_drawImage(GraphicId(ID_Graphic_PanelButton) + 39,
			data.xText + 16 * data.textWidthBlocks + 6, data.yText + offset + 26);
	}
}

void UI_MessageDialog_drawForeground()
{
	// TODO
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.messageId];
	
	if (msg->type == Type_Manual && data.numHistory > 0) {
		Widget_Button_drawImageButtons(
			data.x + 16, data.y + 16 * msg->heightBlocks - 36,
			&imageButtonBack, 1);
		Widget_GameText_draw(12, 0,
			data.x + 52, data.y + 16 * msg->heightBlocks - 31, Font_NormalBlack);
	}

	Widget_Button_drawImageButtons(
		data.x + 16 * msg->widthBlocks - 38,
		data.y + 16 * msg->heightBlocks - 36,
		&imageButtonClose, 1);
	if (data.maxScrollPosition) {
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks - 1,
			data.yText,
			&imageButtonScrollUp, 1);
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks - 1,
			data.yText + 16 * data.textHeightBlocks - 26,
			&imageButtonScrollDown, 1);
	}
}

void UI_MessageDialog_handleMouse()
{
	if (Data_Mouse.scrollDown) {
		buttonScroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		buttonScroll(0, 3);
	}
	// TODO
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.messageId];

	if (Widget_Button_handleImageButtons(
		data.x + 16, data.y + 16 * msg->heightBlocks - 36,
		&imageButtonBack, 1)) {
			return;
	}

	if (Widget_Button_handleImageButtons(
		data.x + 16 * msg->widthBlocks - 38,
		data.y + 16 * msg->heightBlocks - 36,
		&imageButtonClose, 1)) {
			return;
	}
	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks - 1,
		data.yText,
		&imageButtonScrollUp, 1)) {
			return;
	}
	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks - 1,
		data.yText + 16 * data.textHeightBlocks - 26,
		&imageButtonScrollDown, 1)) {
			return;
	}
	int messageId = Widget_RichText_getClickedLink();
	if (messageId >= 0) {
		if (data.numHistory < MAX_HISTORY - 1) {
			data.history[data.numHistory].messageId = data.messageId;
			data.history[data.numHistory].scrollPosition = data.scrollPosition;
			data.numHistory++;
		}
		data.messageId = messageId;
		data.scrollPosition = 0;
		data.numberOfLines = 0;
		data.isDraggingScroll = 0;
		Widget_RichText_clearLinks();
		UI_Window_requestRefresh();
	}
}

static void buttonBack(int param1, int param2)
{
	if (data.numHistory > 0) {
		data.numHistory--;
		data.messageId = data.history[data.numHistory].messageId;
		data.scrollPosition = data.history[data.numHistory].scrollPosition;
		data.numberOfLines = 0;
		data.isDraggingScroll = 0;
		Widget_RichText_clearLinks();
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
	Widget_RichText_clearLinks();
	data.isDraggingScroll = 0;
	UI_Window_requestRefresh();
}

static void buttonClose(int param1, int param2)
{
	// TODO cancel video
	data.showVideo = 0;
	// TODO message_msgAdvisorId = 0
	UI_Window_goBack();
	UI_Window_requestRefresh();
}
