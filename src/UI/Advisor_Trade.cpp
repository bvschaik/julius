#include "Advisors_private.h"
#include "Window.h"
#include "../CityInfo.h"
#include "../Resource.h"
#include "../Data/CityInfo.h"
#include "../Data/Mouse.h"
#include "../Data/Trade.h"

static void buttonPrices(int param1, int param2);
static void buttonEmpire(int param1, int param2);
static void buttonResource(int param1, int param2);

static CustomButton resourceButtons[18] = {
	{400, 398, 600, 421, buttonPrices, Widget_Button_doNothing, 1, 1, 0},
	{100, 398, 300, 421, buttonEmpire, Widget_Button_doNothing, 1, 1, 0},
	{80, 56, 560, 76, buttonResource, Widget_Button_doNothing, 1, 0, 0},
	{80, 78, 560, 98, buttonResource, Widget_Button_doNothing, 1, 1, 0},
	{80, 100, 560, 120, buttonResource, Widget_Button_doNothing, 1, 2, 0},
	{80, 122, 560, 142, buttonResource, Widget_Button_doNothing, 1, 3, 0},
	{80, 144, 560, 164, buttonResource, Widget_Button_doNothing, 1, 4, 0},
	{80, 166, 560, 186, buttonResource, Widget_Button_doNothing, 1, 5, 0},
	{80, 188, 560, 208, buttonResource, Widget_Button_doNothing, 1, 6, 0},
	{80, 210, 560, 230, buttonResource, Widget_Button_doNothing, 1, 7, 0},
	{80, 232, 560, 252, buttonResource, Widget_Button_doNothing, 1, 8, 0},
	{80, 254, 560, 274, buttonResource, Widget_Button_doNothing, 1, 9, 0},
	{80, 276, 560, 296, buttonResource, Widget_Button_doNothing, 1, 10, 0},
	{80, 298, 560, 318, buttonResource, Widget_Button_doNothing, 1, 11, 0},
	{80, 320, 560, 340, buttonResource, Widget_Button_doNothing, 1, 12, 0},
	{80, 342, 560, 362, buttonResource, Widget_Button_doNothing, 1, 13, 0},
	{80, 364, 560, 384, buttonResource, Widget_Button_doNothing, 1, 14, 0}
};

static int focusButtonId;

void UI_Advisor_Trade_drawBackground()
{
	CityInfo_Resource_calculateAvailableResources();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 27);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 4,
		baseOffsetX + 10, baseOffsetY + 10);

	Widget_GameText_draw(54, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);

	Widget_GameText_draw(54, 1, baseOffsetX + 400, baseOffsetY + 38, Font_NormalBlack);
}

void UI_Advisor_Trade_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 52, 36, 21);
	for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++) {
		int offsetY = baseOffsetY + 22 * i;
		int resource = Data_CityInfo_Resource.availableResources[i];
		int graphicOffset = resource + Resource_getGraphicIdOffset(resource, 3);
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + graphicOffset,
			baseOffsetX + 48, offsetY + 54);
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + graphicOffset,
			baseOffsetX + 568, offsetY + 54);
		
		if (focusButtonId - 3 == i) {
			Widget_Panel_drawButtonBorder(baseOffsetX + 80, offsetY + 54, 480, 24, 1);
		}
		Widget_GameText_draw(23, resource, baseOffsetX + 88, offsetY + 61, Font_NormalWhite);
		Widget_Text_drawNumberCentered(Data_CityInfo.resourceStored[resource], '@', " ",
			baseOffsetX + 180, offsetY + 61, 100, Font_NormalWhite);
		if (Data_CityInfo.resourceIndustryMothballed[resource]) {
			Widget_GameText_draw(18, 5, baseOffsetX + 300, offsetY + 61, Font_NormalWhite);
		}
		if (Data_CityInfo.resourceStockpiled[resource]) {
			Widget_GameText_draw(54, 3, baseOffsetX + 380, offsetY + 61, Font_NormalWhite);
		} else if (Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Import) {
			Widget_GameText_draw(54, 5, baseOffsetX + 380, offsetY + 61, Font_NormalWhite);
		} else if (Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Export) {
			Widget_GameText_draw(54, 6, baseOffsetX + 380, offsetY + 61, Font_NormalWhite);
			Widget_Text_drawNumber(Data_CityInfo.resourceTradeExportOver[resource], '@', " ",
				baseOffsetX + 500, offsetY + 61, Font_NormalWhite);
		}
	}

	Widget_Panel_drawButtonBorder(baseOffsetX + 398, baseOffsetY + 396, 200, 24, focusButtonId == 1);
	Widget_GameText_drawCentered(54, 2, baseOffsetX + 400, baseOffsetY + 402, 200, Font_NormalBlack);

	Widget_Panel_drawButtonBorder(baseOffsetX + 98, baseOffsetY + 396, 200, 24, focusButtonId == 2);
	Widget_GameText_drawCentered(54, 30, baseOffsetX + 100, baseOffsetY + 402, 200, Font_NormalBlack);
}

void UI_Advisor_Trade_handleMouse()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY,
		resourceButtons, Data_CityInfo_Resource.numAvailableResources + 2, &focusButtonId);
}

static void buttonPrices(int param1, int param2)
{
	UI_Window_goTo(Window_TradePricesDialog);
}

static void buttonEmpire(int param1, int param2)
{
	UI_Window_goTo(Window_Empire);
}

static void buttonResource(int param1, int param2)
{
	// TODO
}

void UI_TradePricesDialog_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Graphics_shadeRect(baseOffsetX + 33, baseOffsetY + 53, 574, 334, 0);
	Widget_Panel_drawOuterPanel(baseOffsetX + 16, baseOffsetY + 144, 38, 11);
	Widget_GameText_draw(54, 21, baseOffsetX + 26, baseOffsetY + 153, Font_LargeBlack);
	Widget_GameText_draw(54, 22, baseOffsetX + 26, baseOffsetY + 228, Font_NormalBlack);
	Widget_GameText_draw(54, 23, baseOffsetX + 26, baseOffsetY + 253, Font_NormalBlack);
	for (int i = 1; i < 16; i++) {
		int graphicOffset = i + Resource_getGraphicIdOffset(i, 3);
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + graphicOffset,
			baseOffsetX + 126 + 30 * i, baseOffsetY + 194);
		Widget_Text_drawNumberCentered(Data_TradePrices[i].buy, '@', " ",
			baseOffsetX + 120 + 30 * i, baseOffsetY + 229, 30, Font_SmallPlain);
		Widget_Text_drawNumberCentered(Data_TradePrices[i].sell, '@', " ",
			baseOffsetX + 120 + 30 * i, baseOffsetY + 254, 30, Font_SmallPlain);
	}
	Widget_GameText_drawCentered(13, 1, baseOffsetX + 16, baseOffsetY + 296, 608, Font_NormalBlack);
}

void UI_TradePricesDialog_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
		UI_Window_goTo(Window_Advisors);
	}
}
