#include "MessageDialog.h"
#include "Window.h"
#include "AllWindows.h"
#include "Advisors.h"

#include "../Calc.h"
#include "../CityView.h"
#include "../Formation.h"
#include "../Graphics.h"
#include "../PlayerMessage.h"
#include "../Resource.h"
#include "../Video.h"
#include "../Widget.h"

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
static void drawForegroundNoVideo();
static void drawForegroundVideo();
static void buttonBack(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonHelp(int param1, int param2);
static void buttonAdvisor(int param1, int param2);
static void buttonGoToProblem(int param1, int param2);

static ImageButton imageButtonBack = {
	0, 0, 31, 20, ImageButton_Normal, 90, 8, buttonBack, Widget_Button_doNothing, 0, 0, 1
};
static ImageButton imageButtonClose = {
	0, 0, 24, 24, ImageButton_Normal, 134, 4, buttonClose, Widget_Button_doNothing, 0, 0, 1
};
static ImageButton imageButtonGoToProblem = {
	0, 0, 27, 27, ImageButton_Normal, 92, 52, buttonGoToProblem, Widget_Button_doNothing, 1, 0, 1
};
static ImageButton imageButtonHelp = {
	0, 0, 18, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 1
};
static ImageButton imageButtonLabor = {
	0, 0, 27, 27, ImageButton_Normal, 199, 0, buttonAdvisor, Widget_Button_doNothing, Advisor_Labor, 0, 1
};
static ImageButton imageButtonTrade = {
	0, 0, 27, 27, ImageButton_Normal, 199, 12, buttonAdvisor, Widget_Button_doNothing, Advisor_Trade, 0, 1
};
static ImageButton imageButtonPopulation = {
	0, 0, 27, 27, ImageButton_Normal, 199, 15, buttonAdvisor, Widget_Button_doNothing, Advisor_Population, 0, 1
};
static ImageButton imageButtonImperial = {
	0, 0, 27, 27, ImageButton_Normal, 199, 6, buttonAdvisor, Widget_Button_doNothing, Advisor_Imperial, 0, 1
};
static ImageButton imageButtonMilitary = {
	0, 0, 27, 27, ImageButton_Normal, 199, 3, buttonAdvisor, Widget_Button_doNothing, Advisor_Military, 0, 1
};
static ImageButton imageButtonHealth = {
	0, 0, 27, 27, ImageButton_Normal, 199, 18, buttonAdvisor, Widget_Button_doNothing, Advisor_Health, 0, 1
};
static ImageButton imageButtonReligion = {
	0, 0, 27, 27, ImageButton_Normal, 199, 27, buttonAdvisor, Widget_Button_doNothing, Advisor_Religion, 0, 1
};

static struct {
	struct {
		int textId;
		int scrollPosition;
	} history[200];
	int numHistory;

	int textId;
	int dword_7e314c;
	int backgroundIsProvided;
	int showVideo;

	int x;
	int y;
	int xText;
	int yText;
	int textHeightBlocks;
	int textWidthBlocks;
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

void UI_MessageDialog_show(int textId, int backgroundIsProvided)
{
	for (int i = 0; i < MAX_HISTORY; i++) {
		data.history[i].textId = 0;
		data.history[i].scrollPosition = 0;
	}
	data.numHistory = 0;
	Widget_RichText_reset(0);
	data.dword_7e314c = 0;
	data.textId = textId;
	data.backgroundIsProvided = backgroundIsProvided;
	if (Data_Language_Message.index[textId].videoLinkOffset) {
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
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.textId];
	data.x = Data_Screen.offset640x480.x + msg->x;
	data.y = Data_Screen.offset640x480.y + msg->y;
	if (!data.backgroundIsProvided) {
		UI_City_drawBackground();
		UI_City_drawForeground();
	}
	int someOffset = (msg->type == Type_Manual) ? 48 : 32;
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
		if (data.textId) {
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
	data.textHeightBlocks = msg->heightBlocks - 1 - (someOffset + data.yText - data.y) / 16;
	data.textWidthBlocks = Widget_RichText_init(TEXT(msg->contentOffset),
		data.xText, data.yText, msg->widthBlocks - 4, data.textHeightBlocks, 1);

	// content!
	Widget_Panel_drawInnerPanel(data.xText, data.yText, data.textWidthBlocks, data.textHeightBlocks);
	Graphics_setClipRectangle(data.xText + 3, data.yText + 3,
		16 * data.textWidthBlocks - 6, 16 * data.textHeightBlocks - 6);
	Widget_RichText_clearLinks();

	if (msg->type == Type_Message) {
		drawPlayerMessageContent(msg);
	} else {
		Widget_RichText_draw(TEXT(msg->contentOffset),
			data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
			data.textHeightBlocks - 1, 0);
	}
	Graphics_resetClipRectangle();
	Widget_RichText_drawScrollbarDot();
}

static void drawDialogVideo()
{
	Widget_RichText_setFonts(Font_NormalWhite, Font_NormalRed);
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.textId];
	data.x = Data_Screen.offset640x480.x + 32;
	data.y = Data_Screen.offset640x480.y + 28;
	if (!data.backgroundIsProvided) {
		UI_City_drawBackground();
		UI_City_drawForeground();
	}
	Widget_Panel_drawOuterPanel(data.x, data.y, 26, 28);
	Graphics_drawRect(data.x + 7, data.y + 7, 402, 294, Color_Black);
	Widget_RichText_clearLinks();
	
	Video_start(TEXT(msg->videoLinkOffset), data.x + 8, data.y + 8, 0, UI_Window_getId());
	
	Widget_Panel_drawInnerPanel(data.x + 8, data.y + 308, 25, 6);
	Widget_Text_drawCentered(TEXT(msg->titleOffset),
		data.x + 8, data.y + 414, 400, Font_NormalBlack, 0);
	
	int width = Widget_GameText_draw(25, playerMessage.month,
		data.x + 16, data.y + 312, Font_NormalWhite);
	width += Widget_GameText_drawYear(playerMessage.year,
		data.x + 18 + width, data.y + 312, Font_NormalWhite);
	
	if (msg->type == Type_Message && msg->messageType == MessageType_Disaster &&
		data.textId == 251) {
		Widget_GameText_drawNumberWithDescription(8, 0, playerMessage.param1,
			data.x + 90 + width, data.y + 312, Font_NormalWhite);
	} else {
		width += Widget_GameText_draw(63, 5, data.x + 90 + width, data.y + 312, Font_NormalWhite);
		Widget_Text_draw(Data_Settings.playerName, data.x + 90 + width, data.y + 312, Font_NormalWhite, 0);
	}
	data.textHeightBlocks = msg->heightBlocks - 1 - (32 + data.yText - data.y) / 16;
	data.textWidthBlocks = msg->widthBlocks - 4;
	Widget_RichText_draw(TEXT(msg->contentOffset),
		data.x + 16, data.y + 332, 384, data.textHeightBlocks - 1, 0);

	if (msg->type == Type_Message && msg->messageType == MessageType_Imperial) {
		Widget_Text_drawNumber(Data_Scenario.requests.amount[playerMessage.param1],
			'@', " ", data.x + 8, data.y + 384, Font_NormalWhite);
		int resource = Data_Scenario.requests.resourceId[playerMessage.param1];
		Graphics_drawImage(
			GraphicId(ID_Graphic_ResourceIcons) + resource + Resource_getGraphicIdOffset(resource, 3),
			data.x + 70, data.y + 379);
		Widget_GameText_draw(23, resource, data.x + 100, data.y + 384, Font_NormalWhite);
		if (Data_Scenario.requests_state[playerMessage.param1] <= 1) {
			width = Widget_GameText_drawNumberWithDescription(8, 4,
				Data_Scenario.requests_monthsToComply[playerMessage.param1],
				data.x + 200, data.y + 384, Font_NormalWhite);
			Widget_GameText_draw(12, 2, data.x + 200 + width, data.y + 384, Font_NormalWhite);
		}
	}

	drawForegroundVideo();
}

static void drawPlayerMessageContent(struct Data_Language_MessageEntry *msg)
{
	if (msg->messageType != MessageType_Tutorial) {
		int width = Widget_GameText_draw(25, playerMessage.month,
			data.xText + 10, data.yText + 6, Font_NormalWhite);
		width += Widget_GameText_drawYear(playerMessage.year,
			data.xText + 12 + width, data.yText + 6, Font_NormalWhite);
		if (msg->messageType == MessageType_Disaster && playerMessage.param1) {
			if (data.textId == MessageDialog_Theft) {
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
				16 * data.textWidthBlocks, data.textHeightBlocks - 1, 0);
			break;

		case MessageType_Emigration:
			if (Data_CityInfo.populationEmigrationCause >= 1 && Data_CityInfo.populationEmigrationCause <= 5) {
				Widget_GameText_draw(12, Data_CityInfo.populationEmigrationCause + 2,
					data.x + 64, data.yText + 44, Font_NormalWhite);
			}
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MessageType_Tutorial:
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MessageType_TradeChange:
			graphicId = GraphicId(ID_Graphic_ResourceIcons) + playerMessage.param2;
			graphicId += Resource_getGraphicIdOffset(playerMessage.param2, 3);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_GameText_draw(21, Data_Empire_Cities[playerMessage.param1].cityNameId,
				data.x + 100, data.yText + 44, Font_NormalWhite);
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MessageType_PriceChange:
			graphicId = GraphicId(ID_Graphic_ResourceIcons) + playerMessage.param2;
			graphicId += Resource_getGraphicIdOffset(playerMessage.param2, 3);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_Text_drawNumber(playerMessage.param1, '@', " Dn",
				data.x + 100, data.yText + 44, Font_NormalWhite);
			Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		default:
			lines = Widget_RichText_draw(TEXT(msg->contentOffset),
				data.xText + 8, data.yText + 56, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
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

static ImageButton *getAdvisorButton()
{
	switch (playerMessage.messageAdvisor) {
		case MessageAdvisor_Labor:
			return &imageButtonLabor;
		case MessageAdvisor_Trade:
			return &imageButtonTrade;
		case MessageAdvisor_Population:
			return &imageButtonPopulation;
		case MessageAdvisor_Imperial:
			return &imageButtonImperial;
		case MessageAdvisor_Military:
			return &imageButtonMilitary;
		case MessageAdvisor_Health:
			return &imageButtonHealth;
		case MessageAdvisor_Religion:
			return &imageButtonReligion;
		default:
			return &imageButtonHelp;
	}
}

static void drawForegroundVideo()
{
	Widget_Button_drawImageButtons(data.x + 16, data.y + 408, getAdvisorButton(), 1);
	Widget_Button_drawImageButtons(data.x + 372, data.y + 410, &imageButtonClose, 1);
}

static void drawForegroundNoVideo()
{
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.textId];
	
	if (msg->type == Type_Manual && data.numHistory > 0) {
		Widget_Button_drawImageButtons(
			data.x + 16, data.y + 16 * msg->heightBlocks - 36,
			&imageButtonBack, 1);
		Widget_GameText_draw(12, 0,
			data.x + 52, data.y + 16 * msg->heightBlocks - 31, Font_NormalBlack);
	}

	if (msg->type == Type_Message) {
		Widget_Button_drawImageButtons(data.x + 16, data.y + 16 * msg->heightBlocks - 40,
			getAdvisorButton(), 1);
		if (msg->messageType == MessageType_Disaster || msg->messageType == MessageType_Invasion) {
			Widget_Button_drawImageButtons(
				data.x + 64, data.yText + 36, &imageButtonGoToProblem, 1);
		}
	}
	Widget_Button_drawImageButtons(
		data.x + 16 * msg->widthBlocks - 38,
		data.y + 16 * msg->heightBlocks - 36,
		&imageButtonClose, 1);
	Widget_RichText_drawScrollbar();
}

void UI_MessageDialog_drawForeground()
{
	if (data.showVideo) {
		drawForegroundVideo();
	} else {
		drawForegroundNoVideo();
	}
}

void UI_MessageDialog_handleMouse()
{
	if (Data_Mouse.scrollDown) {
		Widget_RichText_scroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		Widget_RichText_scroll(0, 3);
	}
	if (data.showVideo) {
		if (Widget_Button_handleImageButtons(data.x + 16, data.y + 408, getAdvisorButton(), 1, 0)) {
			return;
		}
		if (Widget_Button_handleImageButtons(data.x + 372, data.y + 410, &imageButtonClose, 1, 0)) {
			return;
		}
		return;
	}
	// no video
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.textId];

	if (msg->type == Type_Manual && Widget_Button_handleImageButtons(
		data.x + 16, data.y + 16 * msg->heightBlocks - 36, &imageButtonBack, 1, 0)) {
		return;
	}
	if (msg->type == Type_Message) {
		if (Widget_Button_handleImageButtons(data.x + 16, data.y + 16 * msg->heightBlocks - 40,
			getAdvisorButton(), 1, 0)) {
			return;
		}
		if (msg->messageType == MessageType_Disaster || msg->messageType == MessageType_Invasion) {
			if (Widget_Button_handleImageButtons(data.x + 64, data.yText + 36, &imageButtonGoToProblem, 1, 0)) {
				return;
			}
		}
	}

	if (Widget_Button_handleImageButtons(
		data.x + 16 * msg->widthBlocks - 38,
		data.y + 16 * msg->heightBlocks - 36,
		&imageButtonClose, 1, 0)) {
		return;
	}
	Widget_RichText_handleScrollbar();
	int textId = Widget_RichText_getClickedLink();
	if (textId >= 0) {
		if (data.numHistory < MAX_HISTORY - 1) {
			data.history[data.numHistory].textId = data.textId;
			data.history[data.numHistory].scrollPosition = Widget_RichText_getScrollPosition();
			data.numHistory++;
		}
		data.textId = textId;
		Widget_RichText_reset(0);
		Widget_RichText_clearLinks();
		UI_Window_requestRefresh();
	}
}

static void buttonBack(int param1, int param2)
{
	if (data.numHistory > 0) {
		data.numHistory--;
		data.textId = data.history[data.numHistory].textId;
		Widget_RichText_reset(data.history[data.numHistory].scrollPosition);
		Widget_RichText_clearLinks();
		UI_Window_requestRefresh();
	}
}

void UI_MessageDialog_close()
{
	if (data.showVideo) {
		Video_stop();
	}
	data.showVideo = 0;
	playerMessage.messageAdvisor = 0;
	UI_Window_goBack();
	UI_Window_requestRefresh();
}

static void buttonClose(int param1, int param2)
{
	UI_MessageDialog_close();
}

static void buttonHelp(int param1, int param2)
{
	buttonClose(0, 0);
	UI_MessageDialog_show(MessageDialog_Help, 0);
}

static void buttonAdvisor(int advisor, int param2)
{
	UI_Advisors_goToFromMessage(advisor);
}

static void buttonGoToProblem(int param1, int param2)
{
	struct Data_Language_MessageEntry *msg = &Data_Language_Message.index[data.textId];
	int gridOffset = playerMessage.param2;
	if (msg->messageType == MessageType_Invasion) {
		int invasionGridOffset = Formation_getInvasionGridOffset(playerMessage.param1);
		if (invasionGridOffset > 0) {
			gridOffset = invasionGridOffset;
		}
	}
	if (gridOffset > 0 && gridOffset < 26244) {
		CityView_goToGridOffset(gridOffset);
	}
	UI_Window_goTo(Window_City);
}
