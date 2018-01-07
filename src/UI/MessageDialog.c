#include "MessageDialog.h"
#include "Window.h"
#include "AllWindows.h"
#include "Advisors.h"

#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Screen.h"

#include "city/message.h"
#include "city/view.h"
#include "core/lang.h"
#include "empire/city.h"
#include "figure/formation.h"
#include "graphics/image_button.h"
#include "graphics/video.h"
#include "scenario/property.h"
#include "scenario/request.h"

#define MAX_HISTORY 200

static void drawDialogNormal();
static void drawDialogVideo();
static void drawPlayerMessageContent(const lang_message *msg);
static void drawForegroundNoVideo();
static void drawForegroundVideo();
static void buttonBack(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonHelp(int param1, int param2);
static void buttonAdvisor(int advisor, int param2);
static void buttonGoToProblem(int param1, int param2);

static image_button imageButtonBack = {
	0, 0, 31, 20, IB_NORMAL, 90, 8, buttonBack, button_none, 0, 0, 1
};
static image_button imageButtonClose = {
	0, 0, 24, 24, IB_NORMAL, 134, 4, buttonClose, button_none, 0, 0, 1
};
static image_button imageButtonGoToProblem = {
	0, 0, 27, 27, IB_NORMAL, 92, 52, buttonGoToProblem, button_none, 1, 0, 1
};
static image_button imageButtonHelp = {
	0, 0, 18, 27, IB_NORMAL, 134, 0, buttonHelp, button_none, 1, 0, 1
};
static image_button imageButtonLabor = {
	0, 0, 27, 27, IB_NORMAL, 199, 0, buttonAdvisor, button_none, ADVISOR_LABOR, 0, 1
};
static image_button imageButtonTrade = {
	0, 0, 27, 27, IB_NORMAL, 199, 12, buttonAdvisor, button_none, ADVISOR_TRADE, 0, 1
};
static image_button imageButtonPopulation = {
	0, 0, 27, 27, IB_NORMAL, 199, 15, buttonAdvisor, button_none, ADVISOR_POPULATION, 0, 1
};
static image_button imageButtonImperial = {
	0, 0, 27, 27, IB_NORMAL, 199, 6, buttonAdvisor, button_none, ADVISOR_IMPERIAL, 0, 1
};
static image_button imageButtonMilitary = {
	0, 0, 27, 27, IB_NORMAL, 199, 3, buttonAdvisor, button_none, ADVISOR_MILITARY, 0, 1
};
static image_button imageButtonHealth = {
	0, 0, 27, 27, IB_NORMAL, 199, 18, buttonAdvisor, button_none, ADVISOR_HEALTH, 0, 1
};
static image_button imageButtonReligion = {
	0, 0, 27, 27, IB_NORMAL, 199, 27, buttonAdvisor, button_none, ADVISOR_RELIGION, 0, 1
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
    const lang_message *msg = lang_get_message(textId);
	if (playerMessage.usePopup != 1) {
		data.showVideo = 0;
	} else if (msg->video.text && video_start((char*)msg->video.text)) {
		data.showVideo = 1;
	} else {
		data.showVideo = 0;
	}
	Widget_RichText_clearLinks();
	UI_Window_goTo(Window_MessageDialog);
}

void UI_MessageDialog_init()
{
	if (data.showVideo) {
		video_init();
	}
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
	Widget_RichText_setFonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
	const lang_message *msg = lang_get_message(data.textId);
	data.x = Data_Screen.offset640x480.x + msg->x;
	data.y = Data_Screen.offset640x480.y + msg->y;
	if (!data.backgroundIsProvided) {
		UI_City_drawBackground();
		UI_City_drawForeground();
	}
	int someOffset = (msg->type == TYPE_MANUAL) ? 48 : 32;
	data.xText = data.x + 16;
	Widget_Panel_drawOuterPanel(data.x, data.y, msg->width_blocks, msg->height_blocks);
	// title
	if (msg->title.x) {
		Widget_Text_draw(msg->title.text,
			data.x + msg->title.x, data.y + msg->title.y, FONT_LARGE_BLACK, 0);
		data.yText = data.y + 32;
	} else {
		if (msg->message_type == MESSAGE_TYPE_TUTORIAL) {
			Widget_Text_drawCentered(msg->title.text,
				data.x, data.y + msg->title.y, 16 * msg->width_blocks, FONT_LARGE_BLACK, 0);
		} else {
			Widget_Text_drawCentered(msg->title.text,
				data.x, data.y + 14, 16 * msg->width_blocks, FONT_LARGE_BLACK, 0);
		}
		data.yText = data.y + 48;
	}
	// pictures
	if (msg->image1.id) {
		int graphicId, graphicX, graphicY;
		if (data.textId) {
			graphicId = image_group(GROUP_MESSAGE_IMAGES) + msg->image1.id - 1;
            graphicX = msg->image1.x;
            graphicY = msg->image1.y;
		} else { // message id = 0 ==> about, fixed image position
			graphicX = graphicY = 16;
			graphicId = image_group(GROUP_BIG_PEOPLE);
		}
		Graphics_drawImage(graphicId, data.x + graphicX, data.y + graphicY);
		if (data.y + graphicY + image_get(graphicId)->height + 8 > data.yText) {
			data.yText = data.y + graphicY + image_get(graphicId)->height + 8;
		}
	}
	if (msg->image2.id) {
		int graphicId = image_group(GROUP_MESSAGE_IMAGES) + msg->image2.id - 1;
		Graphics_drawImage(graphicId, data.x + msg->image2.x, data.y + msg->image2.y);
		if (data.y + msg->image2.y + image_get(graphicId)->height + 8 > data.yText) {
			data.yText = data.y + msg->image2.y + image_get(graphicId)->height + 8;
		}
	}
	// subtitle
	if (msg->subtitle.x) {
		int width = 16 * msg->width_blocks - 16 - msg->subtitle.x;
		int height = Widget_Text_drawMultiline(msg->subtitle.text,
			data.x + msg->subtitle.x, data.y + msg->subtitle.y, width,FONT_NORMAL_BLACK);
		if (data.y + msg->subtitle.y + height > data.yText) {
			data.yText = data.y + msg->subtitle.y + height;
		}
	}
	data.textHeightBlocks = msg->height_blocks - 1 - (someOffset + data.yText - data.y) / 16;
	data.textWidthBlocks = Widget_RichText_init(msg->content.text,
		data.xText, data.yText, msg->width_blocks - 4, data.textHeightBlocks, 1);

	// content!
	Widget_Panel_drawInnerPanel(data.xText, data.yText, data.textWidthBlocks, data.textHeightBlocks);
	Graphics_setClipRectangle(data.xText + 3, data.yText + 3,
		16 * data.textWidthBlocks - 6, 16 * data.textHeightBlocks - 6);
	Widget_RichText_clearLinks();

	if (msg->type == TYPE_MESSAGE) {
		drawPlayerMessageContent(msg);
	} else {
		Widget_RichText_draw(msg->content.text,
			data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
			data.textHeightBlocks - 1, 0);
	}
	Graphics_resetClipRectangle();
	Widget_RichText_drawScrollbarDot();
}

static void drawDialogVideo()
{
	Widget_RichText_setFonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
	const lang_message *msg = lang_get_message(data.textId);
	data.x = Data_Screen.offset640x480.x + 32;
	data.y = Data_Screen.offset640x480.y + 28;
	if (!data.backgroundIsProvided) {
		UI_City_drawBackground();
		UI_City_drawForeground();
	}
	Widget_Panel_drawOuterPanel(data.x, data.y, 26, 28);
	Graphics_drawRect(data.x + 7, data.y + 7, 402, 294, COLOR_BLACK);
	Widget_RichText_clearLinks();
	
	Widget_Panel_drawInnerPanel(data.x + 8, data.y + 308, 25, 6);
	Widget_Text_drawCentered(msg->title.text,
		data.x + 8, data.y + 414, 400, FONT_NORMAL_BLACK, 0);
	
	int width = Widget_GameText_draw(25, playerMessage.month,
		data.x + 16, data.y + 312, FONT_NORMAL_WHITE);
	width += Widget_GameText_drawYear(playerMessage.year,
		data.x + 18 + width, data.y + 312, FONT_NORMAL_WHITE);
	
	if (msg->type == TYPE_MESSAGE && msg->message_type == MESSAGE_TYPE_DISASTER &&
		data.textId == 251) {
		Widget_GameText_drawNumberWithDescription(8, 0, playerMessage.param1,
			data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE);
	} else {
		width += Widget_GameText_draw(63, 5, data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE);
		Widget_Text_draw(scenario_player_name(), data.x + 90 + width, data.y + 312, FONT_NORMAL_WHITE, 0);
	}
	data.textHeightBlocks = msg->height_blocks - 1 - (32 + data.yText - data.y) / 16;
	data.textWidthBlocks = msg->width_blocks - 4;
	Widget_RichText_draw(msg->content.text,
		data.x + 16, data.y + 332, 384, data.textHeightBlocks - 1, 0);

	if (msg->type == TYPE_MESSAGE && msg->message_type == MESSAGE_TYPE_IMPERIAL) {
        const scenario_request *request = scenario_request_get(playerMessage.param1);
		Widget_Text_drawNumber(request->amount,
			'@', " ", data.x + 8, data.y + 384, FONT_NORMAL_WHITE);
		Graphics_drawImage(
			image_group(GROUP_RESOURCE_ICONS) + request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON),
			data.x + 70, data.y + 379);
		Widget_GameText_draw(23, request->resource, data.x + 100, data.y + 384, FONT_NORMAL_WHITE);
		if (request->state == REQUEST_STATE_NORMAL || request->state == REQUEST_STATE_OVERDUE) {
			width = Widget_GameText_drawNumberWithDescription(8, 4, request->months_to_comply,
				data.x + 200, data.y + 384, FONT_NORMAL_WHITE);
			Widget_GameText_draw(12, 2, data.x + 200 + width, data.y + 384, FONT_NORMAL_WHITE);
		}
	}

	drawForegroundVideo();
}

static void drawPlayerMessageContent(const lang_message *msg)
{
	if (msg->message_type != MESSAGE_TYPE_TUTORIAL) {
		int width = Widget_GameText_draw(25, playerMessage.month,
			data.xText + 10, data.yText + 6, FONT_NORMAL_WHITE);
		width += Widget_GameText_drawYear(playerMessage.year,
			data.xText + 12 + width, data.yText + 6, FONT_NORMAL_WHITE);
		if (msg->message_type == MESSAGE_TYPE_DISASTER && playerMessage.param1) {
			if (data.textId == MessageDialog_Theft) {
				// param1 = denarii
				Widget_GameText_drawNumberWithDescription(8, 0, playerMessage.param1,
					data.x + 240, data.yText + 6, FONT_NORMAL_WHITE);
			} else {
				// param1 = building type
				Widget_GameText_draw(41, playerMessage.param1,
					data.x + 240, data.yText + 6, FONT_NORMAL_WHITE);
			}
		} else {
			width += Widget_GameText_draw(63, 5,
				data.xText + width + 80, data.yText + 6, FONT_NORMAL_WHITE);
			Widget_Text_draw(scenario_player_name(),
				data.xText + width + 80, data.yText + 6, FONT_NORMAL_WHITE, 0);
		}
	}
	int graphicId;
	int lines = 0;
	switch (msg->message_type) {
		case MESSAGE_TYPE_DISASTER:
		case MESSAGE_TYPE_INVASION:
			Widget_GameText_draw(12, 1, data.x + 100, data.yText + 44, FONT_NORMAL_WHITE);
			Widget_RichText_draw(msg->content.text, data.xText + 8, data.yText + 86,
				16 * data.textWidthBlocks, data.textHeightBlocks - 1, 0);
			break;

		case MESSAGE_TYPE_EMIGRATION:
			if (Data_CityInfo.populationEmigrationCause >= 1 && Data_CityInfo.populationEmigrationCause <= 5) {
				Widget_GameText_draw(12, Data_CityInfo.populationEmigrationCause + 2,
					data.x + 64, data.yText + 44, FONT_NORMAL_WHITE);
			}
			Widget_RichText_draw(msg->content.text,
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MESSAGE_TYPE_TUTORIAL:
			Widget_RichText_draw(msg->content.text,
				data.xText + 8, data.yText + 6, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MESSAGE_TYPE_TRADE_CHANGE:
			graphicId = image_group(GROUP_RESOURCE_ICONS) + playerMessage.param2;
			graphicId += resource_image_offset(playerMessage.param2, RESOURCE_IMAGE_ICON);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_GameText_draw(21, empire_city_get(playerMessage.param1)->name_id,
				data.x + 100, data.yText + 44, FONT_NORMAL_WHITE);
			Widget_RichText_draw(msg->content.text,
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		case MESSAGE_TYPE_PRICE_CHANGE:
			graphicId = image_group(GROUP_RESOURCE_ICONS) + playerMessage.param2;
			graphicId += resource_image_offset(playerMessage.param2, RESOURCE_IMAGE_ICON);
			Graphics_drawImage(graphicId, data.x + 64, data.yText + 40);
			Widget_Text_drawMoney(playerMessage.param1,
				data.x + 100, data.yText + 44, FONT_NORMAL_WHITE);
			Widget_RichText_draw(msg->content.text,
				data.xText + 8, data.yText + 86, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
			break;

		default:
			lines = Widget_RichText_draw(msg->content.text,
				data.xText + 8, data.yText + 56, 16 * data.textWidthBlocks - 16,
				data.textHeightBlocks - 1, 0);
	}
	if (msg->message_type == MESSAGE_TYPE_IMPERIAL) {
        const scenario_request *request = scenario_request_get(playerMessage.param1);
		int yOffset = data.yText + 86 + lines * 16;
		Widget_Text_drawNumber(request->amount,
			'@', " ", data.xText + 8, yOffset, FONT_NORMAL_WHITE);
		graphicId = image_group(GROUP_RESOURCE_ICONS) + request->resource;
		graphicId += resource_image_offset(request->resource, RESOURCE_IMAGE_ICON);
		Graphics_drawImage(graphicId, data.xText + 70, yOffset - 5);
		Widget_GameText_draw(23, request->resource,
			data.xText + 100, yOffset, FONT_NORMAL_WHITE);
		if (request->state == REQUEST_STATE_NORMAL || request->state == REQUEST_STATE_OVERDUE) {
			int width = Widget_GameText_drawNumberWithDescription(8, 4,
				request->months_to_comply,
				data.xText + 200, yOffset, FONT_NORMAL_WHITE);
			Widget_GameText_draw(12, 2, data.xText + 200 + width, yOffset, FONT_NORMAL_WHITE);
		}
	}
}

static image_button *getAdvisorButton()
{
	switch (playerMessage.messageAdvisor) {
		case MESSAGE_ADVISOR_LABOR:
			return &imageButtonLabor;
		case MESSAGE_ADVISOR_TRADE:
			return &imageButtonTrade;
		case MESSAGE_ADVISOR_POPULATION:
			return &imageButtonPopulation;
		case MESSAGE_ADVISOR_IMPERIAL:
			return &imageButtonImperial;
		case MESSAGE_ADVISOR_MILITARY:
			return &imageButtonMilitary;
		case MESSAGE_ADVISOR_HEALTH:
			return &imageButtonHealth;
		case MESSAGE_ADVISOR_RELIGION:
			return &imageButtonReligion;
		default:
			return &imageButtonHelp;
	}
}

static void drawForegroundVideo()
{
	video_draw(data.x + 8, data.y + 8);
	image_buttons_draw(data.x + 16, data.y + 408, getAdvisorButton(), 1);
	image_buttons_draw(data.x + 372, data.y + 410, &imageButtonClose, 1);
}

static void drawForegroundNoVideo()
{
	const lang_message *msg = lang_get_message(data.textId);
	
	if (msg->type == TYPE_MANUAL && data.numHistory > 0) {
		image_buttons_draw(
			data.x + 16, data.y + 16 * msg->height_blocks - 36,
			&imageButtonBack, 1);
		Widget_GameText_draw(12, 0,
			data.x + 52, data.y + 16 * msg->height_blocks - 31, FONT_NORMAL_BLACK);
	}

	if (msg->type == TYPE_MESSAGE) {
		image_buttons_draw(data.x + 16, data.y + 16 * msg->height_blocks - 40,
			getAdvisorButton(), 1);
		if (msg->message_type == MESSAGE_TYPE_DISASTER || msg->message_type == MESSAGE_TYPE_INVASION) {
			image_buttons_draw(
				data.x + 64, data.yText + 36, &imageButtonGoToProblem, 1);
		}
	}
	image_buttons_draw(
		data.x + 16 * msg->width_blocks - 38,
		data.y + 16 * msg->height_blocks - 36,
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

void UI_MessageDialog_handleMouse(const mouse *m)
{
	if (m->scrolled == SCROLL_DOWN) {
		Widget_RichText_scroll(1, 3);
	} else if (m->scrolled == SCROLL_UP) {
		Widget_RichText_scroll(0, 3);
	}
	if (data.showVideo) {
		if (image_buttons_handle_mouse(m, data.x + 16, data.y + 408, getAdvisorButton(), 1, 0)) {
			return;
		}
		if (image_buttons_handle_mouse(m, data.x + 372, data.y + 410, &imageButtonClose, 1, 0)) {
			return;
		}
		return;
	}
	// no video
	const lang_message *msg = lang_get_message(data.textId);

	if (msg->type == TYPE_MANUAL && image_buttons_handle_mouse(
		m, data.x + 16, data.y + 16 * msg->height_blocks - 36, &imageButtonBack, 1, 0)) {
		return;
	}
	if (msg->type == TYPE_MESSAGE) {
		if (image_buttons_handle_mouse(m, data.x + 16, data.y + 16 * msg->height_blocks - 40,
			getAdvisorButton(), 1, 0)) {
			return;
		}
		if (msg->message_type == MESSAGE_TYPE_DISASTER || msg->message_type == MESSAGE_TYPE_INVASION) {
			if (image_buttons_handle_mouse(m, data.x + 64, data.yText + 36, &imageButtonGoToProblem, 1, 0)) {
				return;
			}
		}
	}

	if (image_buttons_handle_mouse(m,
		data.x + 16 * msg->width_blocks - 38,
		data.y + 16 * msg->height_blocks - 36,
		&imageButtonClose, 1, 0)) {
		return;
	}
	Widget_RichText_handleScrollbar(m);
	int textId = Widget_RichText_getClickedLink(m);
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

static void cleanup()
{
	if (data.showVideo) {
		video_stop();
	}
	data.showVideo = 0;
	playerMessage.messageAdvisor = 0;
}

void UI_MessageDialog_close()
{
	cleanup();
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
	cleanup();
	UI_Advisors_goToFromMessage(advisor);
}

static void buttonGoToProblem(int param1, int param2)
{
	cleanup();
	const lang_message *msg = lang_get_message(data.textId);
	int gridOffset = playerMessage.param2;
	if (msg->message_type == MESSAGE_TYPE_INVASION) {
		int invasionGridOffset = formation_grid_offset_for_invasion(playerMessage.param1);
		if (invasionGridOffset > 0) {
			gridOffset = invasionGridOffset;
		}
	}
	if (gridOffset > 0 && gridOffset < 26244) {
		city_view_go_to_grid_offset(gridOffset);
	}
	UI_Window_goTo(Window_City);
}
