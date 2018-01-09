#include "Advisors_private.h"
#include "Tooltip.h"
#include "Window.h"
#include "MessageDialog.h"

#include "building/count.h"
#include "city/constants.h"
#include "city/resource.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/trade_prices.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "scenario/building.h"

static void buttonPrices(int param1, int param2);
static void buttonEmpire(int param1, int param2);
static void buttonResource(int resourceIndex, int param2);

static void resourceSettingsHelp(int param1, int param2);
static void resourceSettingsOk(int param1, int param2);
static void resourceSettingsExportUpDown(int isDown, int param2);

static void resourceSettingsToggleIndustry(int param1, int param2);
static void resourceSettingsToggleTrade(int param1, int param2);
static void resourceSettingsToggleStockpile(int param1, int param2);

static generic_button resourceButtons[] = {
	{400, 398, 600, 421, GB_IMMEDIATE, buttonPrices, button_none, 1, 0},
	{100, 398, 300, 421, GB_IMMEDIATE, buttonEmpire, button_none, 1, 0},
	{80, 56, 560, 76, GB_IMMEDIATE, buttonResource, button_none, 0, 0},
	{80, 78, 560, 98, GB_IMMEDIATE, buttonResource, button_none, 1, 0},
	{80, 100, 560, 120, GB_IMMEDIATE, buttonResource, button_none, 2, 0},
	{80, 122, 560, 142, GB_IMMEDIATE, buttonResource, button_none, 3, 0},
	{80, 144, 560, 164, GB_IMMEDIATE, buttonResource, button_none, 4, 0},
	{80, 166, 560, 186, GB_IMMEDIATE, buttonResource, button_none, 5, 0},
	{80, 188, 560, 208, GB_IMMEDIATE, buttonResource, button_none, 6, 0},
	{80, 210, 560, 230, GB_IMMEDIATE, buttonResource, button_none, 7, 0},
	{80, 232, 560, 252, GB_IMMEDIATE, buttonResource, button_none, 8, 0},
	{80, 254, 560, 274, GB_IMMEDIATE, buttonResource, button_none, 9, 0},
	{80, 276, 560, 296, GB_IMMEDIATE, buttonResource, button_none, 10, 0},
	{80, 298, 560, 318, GB_IMMEDIATE, buttonResource, button_none, 11, 0},
	{80, 320, 560, 340, GB_IMMEDIATE, buttonResource, button_none, 12, 0},
	{80, 342, 560, 362, GB_IMMEDIATE, buttonResource, button_none, 13, 0},
	{80, 364, 560, 384, GB_IMMEDIATE, buttonResource, button_none, 14, 0}
};

static image_button resourceImageButtons[] = {
	{58, 332, 27, 27, IB_NORMAL, 134, 0, resourceSettingsHelp, button_none, 0, 0, 1},
	{558, 335, 24, 24, IB_NORMAL, 134, 4, resourceSettingsOk, button_none, 0, 0, 1}
};

static arrow_button resourceArrowButtons[] = {
	{314, 215, 17, 24, resourceSettingsExportUpDown, 1, 0},
	{338, 215, 15, 24, resourceSettingsExportUpDown, 0, 0}
};

static generic_button resourceCustomButtons[] = {
	{98, 250, 530, 280, GB_IMMEDIATE, resourceSettingsToggleIndustry, button_none, 0, 0},
	{98, 212, 530, 242, GB_IMMEDIATE, resourceSettingsToggleTrade, button_none, 0, 0},
	{98, 288, 530, 338, GB_IMMEDIATE, resourceSettingsToggleStockpile, button_none, 0, 0},
};

static int focusButtonId;

static int selectedResourceId;
static int resourceFocusButtonId;

void UI_Advisor_Trade_drawBackground(int *advisorHeight)
{
	city_resource_determine_available();

    graphics_in_dialog();

	*advisorHeight = 27;
	outer_panel_draw(0, 0, 40, *advisorHeight);
	image_draw(image_group(GROUP_ADVISOR_ICONS) + 4, 10, 10);

	lang_text_draw(54, 0, 60, 12, FONT_LARGE_BLACK);
	lang_text_draw(54, 1, 400, 38, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

void UI_Advisor_Trade_drawForeground()
{
    graphics_in_dialog();

	inner_panel_draw(32, 52, 36, 21);
	for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++) {
		int offsetY = 22 * i;
		int resource = Data_CityInfo_Resource.availableResources[i];
		int graphicOffset = resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON);
		image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset, 48, offsetY + 54);
		image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset, 568, offsetY + 54);
		
		if (focusButtonId - 3 == i) {
			button_border_draw(80, offsetY + 54, 480, 24, 1);
		}
		lang_text_draw(23, resource, 88, offsetY + 61, FONT_NORMAL_WHITE);
		text_draw_number_centered(Data_CityInfo.resourceStored[resource],
			180, offsetY + 61, 100, FONT_NORMAL_WHITE);
		if (Data_CityInfo.resourceIndustryMothballed[resource]) {
			lang_text_draw(18, 5, 300, offsetY + 61, FONT_NORMAL_WHITE);
		}
		if (Data_CityInfo.resourceStockpiled[resource]) {
			lang_text_draw(54, 3, 380, offsetY + 61, FONT_NORMAL_WHITE);
		} else if (Data_CityInfo.resourceTradeStatus[resource] == TRADE_STATUS_IMPORT) {
			lang_text_draw(54, 5, 380, offsetY + 61, FONT_NORMAL_WHITE);
		} else if (Data_CityInfo.resourceTradeStatus[resource] == TRADE_STATUS_EXPORT) {
			lang_text_draw(54, 6, 380, offsetY + 61, FONT_NORMAL_WHITE);
			text_draw_number(Data_CityInfo.resourceTradeExportOver[resource], '@', " ",
				500, offsetY + 61, FONT_NORMAL_WHITE);
		}
	}

	button_border_draw(398, 396, 200, 24, focusButtonId == 1);
	lang_text_draw_centered(54, 2, 400, 402, 200, FONT_NORMAL_BLACK);

	button_border_draw(98, 396, 200, 24, focusButtonId == 2);
	lang_text_draw_centered(54, 30, 100, 402, 200, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

void UI_Advisor_Trade_handleMouse(const mouse *m)
{
	generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0,
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

static void buttonResource(int resourceIndex, int param2)
{
	selectedResourceId = Data_CityInfo_Resource.availableResources[resourceIndex];
	UI_Window_goTo(Window_ResourceSettingsDialog);
}

int UI_Advisor_Trade_getTooltip()
{
	if (focusButtonId == 1) {
		return 106;
	} else if (focusButtonId == 2) {
		return 41; // FIXED used to say 'select this resource'
	} else if (focusButtonId) {
		return 107;
	} else {
		return 0;
	}
}


void UI_TradePricesDialog_drawBackground()
{
    graphics_in_dialog();

	graphics_shade_rect(33, 53, 574, 334, 0);
	outer_panel_draw(16, 144, 38, 11);
	lang_text_draw(54, 21, 26, 153, FONT_LARGE_BLACK);
	lang_text_draw(54, 22, 26, 228, FONT_NORMAL_BLACK);
	lang_text_draw(54, 23, 26, 253, FONT_NORMAL_BLACK);
	for (int i = 1; i < 16; i++) {
		int graphicOffset = i + resource_image_offset(i, RESOURCE_IMAGE_ICON);
		image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset,
			126 + 30 * i, 194);
		text_draw_number_centered(trade_price_buy(i),
			120 + 30 * i, 229, 30, FONT_SMALL_PLAIN);
		text_draw_number_centered(trade_price_sell(i),
			120 + 30 * i, 254, 30, FONT_SMALL_PLAIN);
	}
	lang_text_draw_centered(13, 1, 16, 296, 608, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

void UI_TradePricesDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	}
}

static int getTradePriceTooltipResource(struct TooltipContext *c)
{
	int xBase = screen_dialog_offset_x() + 124;
	int y = screen_dialog_offset_y() + 192;
	int xMouse = c->mouse_x;
	int yMouse = c->mouse_y;
	
	for (int i = 1; i < 16; i++) {
		int x = xBase + 30 * i;
		if (x <= xMouse && x + 24 > xMouse && y <= yMouse && y + 24 > yMouse) {
			return i;
		}
	}
	return 0;
}

void UI_TradePricesDialog_getTooltip(struct TooltipContext *c)
{
	int resource = getTradePriceTooltipResource(c);
	if (!resource) {
		return;
	}
	c->type = TooltipType_Button;
	c->textId = 131 + resource;
}

void UI_ResourceSettingsDialog_drawBackground()
{
	int dummy;
	UI_Advisor_Trade_drawBackground(&dummy);
	UI_Advisor_Trade_drawForeground();
}

void UI_ResourceSettingsDialog_drawForeground()
{
    graphics_in_dialog();

	outer_panel_draw(48, 128, 34, 15);
	int graphicOffset = selectedResourceId + resource_image_offset(selectedResourceId, RESOURCE_IMAGE_ICON);
	image_draw(image_group(GROUP_RESOURCE_ICONS) + graphicOffset, 58, 136);

	lang_text_draw(23, selectedResourceId, 92, 137, FONT_LARGE_BLACK);

	if (empire_can_produce_resource(selectedResourceId)) {
        int totalBuildings = building_count_industry_total(selectedResourceId);
        int activeBuildings = building_count_industry_active(selectedResourceId);
		if (building_count_industry_total(selectedResourceId) <= 0) {
			lang_text_draw(54, 7, 98, 172, FONT_NORMAL_BLACK);
		} else if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId] == 1) {
			int width = text_draw_number(totalBuildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
			if (totalBuildings == 1) {
				lang_text_draw(54, 10, 98 + width, 172, FONT_NORMAL_BLACK);
			} else {
				lang_text_draw(54, 11, 98 + width, 172, FONT_NORMAL_BLACK);
			}
		} else if (totalBuildings == activeBuildings) {
			// not mothballed, all working
			int width = text_draw_number(totalBuildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
			if (totalBuildings == 1) {
				lang_text_draw(54, 8, 98 + width, 172, FONT_NORMAL_BLACK);
			} else {
				lang_text_draw(54, 9, 98 + width, 172, FONT_NORMAL_BLACK);
			}
		} else {
			// not mothballed, some working
			int width = text_draw_number(activeBuildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
			width += lang_text_draw(54, 12, 98 + width, 172, FONT_NORMAL_BLACK);
			width += text_draw_number(totalBuildings - activeBuildings, '@', " ", 98 + width, 172, FONT_NORMAL_BLACK);
			if (activeBuildings == 1) {
				lang_text_draw(54, 13, 98 + width, 172, FONT_NORMAL_BLACK);
			} else {
				lang_text_draw(54, 14, 98 + width, 172, FONT_NORMAL_BLACK);
			}
		}
	} else if (selectedResourceId != RESOURCE_MEAT || !scenario_building_allowed(BUILDING_WHARF)) {
		// we cannot produce this good
		lang_text_draw(54, 25, 98, 172, FONT_NORMAL_BLACK);
	}

	int width = lang_text_draw_amount(8, 10, Data_CityInfo.resourceStored[selectedResourceId], 98, 192, FONT_NORMAL_BLACK);
	lang_text_draw(54, 15, 98 + width, 192, FONT_NORMAL_BLACK);

	int tradeFlags = TRADE_STATUS_NONE;
	if (empire_can_import_resource(selectedResourceId)) {
		tradeFlags |= TRADE_STATUS_IMPORT;
	}
	if (empire_can_export_resource(selectedResourceId)) {
		tradeFlags |= TRADE_STATUS_EXPORT;
	}
	if (!tradeFlags) {
		lang_text_draw(54, 24, 98, 212, FONT_NORMAL_BLACK);
	} else {
		button_border_draw(98, 212, 432, 30, resourceFocusButtonId == 2);
		switch (Data_CityInfo.resourceTradeStatus[selectedResourceId]) {
			case TRADE_STATUS_NONE:
				lang_text_draw_centered(54, 18, 114, 221, 400, FONT_NORMAL_BLACK);
				break;
			case TRADE_STATUS_IMPORT:
				lang_text_draw_centered(54, 19, 114, 221, 400, FONT_NORMAL_BLACK);
				break;
			case TRADE_STATUS_EXPORT:
				lang_text_draw_centered(54, 20, 114, 221, 200, FONT_NORMAL_BLACK);
				break;
		}
	}

	if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_EXPORT) {
		lang_text_draw_amount(8, 10, Data_CityInfo.resourceTradeExportOver[selectedResourceId],
			386, 221, FONT_NORMAL_BLACK);
	}

	if (building_count_industry_total(selectedResourceId) > 0) {
		button_border_draw(98, 250, 432, 30, resourceFocusButtonId == 1);
		if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId]) {
			lang_text_draw_centered(54, 17, 114, 259, 400, FONT_NORMAL_BLACK);
		} else {
			lang_text_draw_centered(54, 16, 114, 259, 400, FONT_NORMAL_BLACK);
		}
	}

	button_border_draw(98, 288, 432, 50, resourceFocusButtonId == 3);
	if (Data_CityInfo.resourceStockpiled[selectedResourceId]) {
		lang_text_draw_centered(54, 26, 114, 296, 400, FONT_NORMAL_BLACK);
		lang_text_draw_centered(54, 27, 114, 316, 400, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_centered(54, 28, 114, 296, 400, FONT_NORMAL_BLACK);
		lang_text_draw_centered(54, 29, 114, 316, 400, FONT_NORMAL_BLACK);
	}

	image_buttons_draw(0, 0, resourceImageButtons, 2);
	if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_EXPORT) {
		arrow_buttons_draw(0, 0, resourceArrowButtons, 2);
	}
	graphics_reset_dialog();
}

void UI_ResourceSettingsDialog_handleMouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else if (image_buttons_handle_mouse(m_dialog, 0, 0, resourceImageButtons, 2, 0)) {
		return;
	} else if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_EXPORT &&
			arrow_buttons_handle_mouse(m_dialog, 0, 0, resourceArrowButtons, 2)) {
		return;
	} else {
		generic_buttons_handle_mouse(m_dialog, 0, 0, resourceCustomButtons, 3, &resourceFocusButtonId);
	}
}

static void resourceSettingsHelp(int param1, int param2)
{
	UI_MessageDialog_show(MessageDialog_Industry, 1);
}

static void resourceSettingsOk(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void resourceSettingsExportUpDown(int isDown, int param2)
{
	if (isDown) {
		--Data_CityInfo.resourceTradeExportOver[selectedResourceId];
	} else {
		++Data_CityInfo.resourceTradeExportOver[selectedResourceId];
	}
	Data_CityInfo.resourceTradeExportOver[selectedResourceId] =
        calc_bound(Data_CityInfo.resourceTradeExportOver[selectedResourceId], 0, 100);
}

static void resourceSettingsToggleIndustry(int param1, int param2)
{
	if (building_count_industry_total(selectedResourceId) > 0) {
		if (Data_CityInfo.resourceIndustryMothballed[selectedResourceId]) {
			Data_CityInfo.resourceIndustryMothballed[selectedResourceId] = 0;
		} else {
			Data_CityInfo.resourceIndustryMothballed[selectedResourceId] = 1;
		}
	}
}

static void resourceSettingsToggleTrade(int param1, int param2)
{
	++Data_CityInfo.resourceTradeStatus[selectedResourceId];
	if (Data_CityInfo.resourceTradeStatus[selectedResourceId] > TRADE_STATUS_EXPORT) {
		Data_CityInfo.resourceTradeStatus[selectedResourceId] = TRADE_STATUS_NONE;
	}

	if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_IMPORT &&
		!empire_can_import_resource(selectedResourceId)) {
		Data_CityInfo.resourceTradeStatus[selectedResourceId] = TRADE_STATUS_EXPORT;
	}
	if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_EXPORT &&
		!empire_can_export_resource(selectedResourceId)) {
		Data_CityInfo.resourceTradeStatus[selectedResourceId] = TRADE_STATUS_NONE;
	}
}

static void resourceSettingsToggleStockpile(int param1, int param2)
{
	if (Data_CityInfo.resourceStockpiled[selectedResourceId]) {
		Data_CityInfo.resourceStockpiled[selectedResourceId] = 0;
	} else {
		Data_CityInfo.resourceStockpiled[selectedResourceId] = 1;
		if (Data_CityInfo.resourceTradeStatus[selectedResourceId] == TRADE_STATUS_EXPORT) {
			Data_CityInfo.resourceTradeStatus[selectedResourceId] = TRADE_STATUS_NONE;
		}
	}
}
