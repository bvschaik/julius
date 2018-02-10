#include "BuildingInfo.h"

#include "../Data/CityInfo.h"

#include "building/building.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "figure/figure.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "window/building_info.h"

static void toggleResourceState(int index, int param2);
static void granaryOrders(int index, int param2);
static void warehouseOrders(int index, int param2);

static generic_button gotoOrdersButtons[] = {
	{0, 0, 304, 20, GB_IMMEDIATE, window_building_info_show_storage_orders, button_none, 0, 0}
};

static generic_button ordersResourceButtons[] = {
	{0, 0, 210, 22, GB_IMMEDIATE, toggleResourceState, button_none, 1, 0},
	{0, 22, 210, 44, GB_IMMEDIATE, toggleResourceState, button_none, 2, 0},
	{0, 44, 210, 66, GB_IMMEDIATE, toggleResourceState, button_none, 3, 0},
	{0, 66, 210, 88, GB_IMMEDIATE, toggleResourceState, button_none, 4, 0},
	{0, 88, 210, 110, GB_IMMEDIATE, toggleResourceState, button_none, 5, 0},
	{0, 110, 210, 132, GB_IMMEDIATE, toggleResourceState, button_none, 6, 0},
	{0, 132, 210, 154, GB_IMMEDIATE, toggleResourceState, button_none, 7, 0},
	{0, 154, 210, 176, GB_IMMEDIATE, toggleResourceState, button_none, 8, 0},
	{0, 176, 210, 198, GB_IMMEDIATE, toggleResourceState, button_none, 9, 0},
	{0, 198, 210, 220, GB_IMMEDIATE, toggleResourceState, button_none, 10, 0},
	{0, 220, 210, 242, GB_IMMEDIATE, toggleResourceState, button_none, 11, 0},
	{0, 242, 210, 264, GB_IMMEDIATE, toggleResourceState, button_none, 12, 0},                                                   
	{0, 264, 210, 286, GB_IMMEDIATE, toggleResourceState, button_none, 13, 0},
	{0, 286, 210, 308, GB_IMMEDIATE, toggleResourceState, button_none, 14, 0},
	{0, 308, 210, 330, GB_IMMEDIATE, toggleResourceState, button_none, 15, 0},
};

static generic_button granaryOrderButtons[] = {
	{0, 0, 304, 20, GB_IMMEDIATE, granaryOrders, button_none, 0, 0},
};

static generic_button warehouseOrderButtons[] = {
	{0, 0, 304, 20, GB_IMMEDIATE, warehouseOrders, button_none, 0, 0},
	{0, -22, 304, 20, GB_IMMEDIATE, warehouseOrders, button_none, 1, 0},
};

static int focusButtonId = 0;
static int ordersFocusButtonId = 0;
static int resourceFocusButtonId = 0;
static int buildingId;

void UI_BuildingInfo_drawMarket(BuildingInfoContext *c)
{
	c->helpId = 2;
	window_building_play_sound(c, "wavs/market.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(97, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		window_building_draw_description(c, 69, 25);
	} else if (b->numWorkers <= 0) {
		window_building_draw_description(c, 97, 2);
	} else {
		int graphicId = image_group(GROUP_RESOURCE_ICONS);
		if (b->data.market.inventory[INVENTORY_WHEAT] || b->data.market.inventory[INVENTORY_VEGETABLES] ||
			b->data.market.inventory[INVENTORY_FRUIT] || b->data.market.inventory[INVENTORY_MEAT]) {
			// food stocks
			image_draw(graphicId + RESOURCE_WHEAT,
				c->xOffset + 32, c->yOffset + 64);
			text_draw_number(b->data.market.inventory[INVENTORY_WHEAT], '@', " ",
				c->xOffset + 64, c->yOffset + 70, FONT_NORMAL_BLACK);
			image_draw(graphicId + RESOURCE_VEGETABLES,
				c->xOffset + 142, c->yOffset + 64);
			text_draw_number(b->data.market.inventory[INVENTORY_VEGETABLES], '@', " ",
				c->xOffset + 174, c->yOffset + 70, FONT_NORMAL_BLACK);
			image_draw(graphicId + RESOURCE_FRUIT,
				c->xOffset + 252, c->yOffset + 64);
			text_draw_number(b->data.market.inventory[INVENTORY_FRUIT], '@', " ",
				c->xOffset + 284, c->yOffset + 70, FONT_NORMAL_BLACK);
			image_draw(graphicId + RESOURCE_MEAT +
				resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
				c->xOffset + 362, c->yOffset + 64);
			text_draw_number(b->data.market.inventory[INVENTORY_MEAT], '@', " ",
				c->xOffset + 394, c->yOffset + 70, FONT_NORMAL_BLACK);
		} else {
			window_building_draw_description_at(c, 48, 97, 4);
		}
		// good stocks
		image_draw(graphicId + RESOURCE_POTTERY,
			c->xOffset + 32, c->yOffset + 104);
		text_draw_number(b->data.market.inventory[INVENTORY_POTTERY], '@', " ",
			c->xOffset + 64, c->yOffset + 110, FONT_NORMAL_BLACK);
		image_draw(graphicId + RESOURCE_FURNITURE,
			c->xOffset + 142, c->yOffset + 104);
		text_draw_number(b->data.market.inventory[INVENTORY_FURNITURE], '@', " ",
			c->xOffset + 174, c->yOffset + 110, FONT_NORMAL_BLACK);
		image_draw(graphicId + RESOURCE_OIL,
			c->xOffset + 252, c->yOffset + 104);
		text_draw_number(b->data.market.inventory[INVENTORY_OIL], '@', " ",
			c->xOffset + 284, c->yOffset + 110, FONT_NORMAL_BLACK);
		image_draw(graphicId + RESOURCE_WINE,
			c->xOffset + 362, c->yOffset + 104);
		text_draw_number(b->data.market.inventory[INVENTORY_WINE], '@', " ",
			c->xOffset + 394, c->yOffset + 110, FONT_NORMAL_BLACK);
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, 142);
}

void UI_BuildingInfo_drawGranary(BuildingInfoContext *c)
{
	c->helpId = 3;
	window_building_play_sound(c, "wavs/granary.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(98, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		window_building_draw_description_at(c, 40, 69, 25);
	} else if (scenario_property_rome_supplies_wheat()) {
		window_building_draw_description_at(c, 40, 98, 4);
	} else {
		int totalStored = 0;
		for (int i = RESOURCE_WHEAT; i <= RESOURCE_MEAT; i++) {
			totalStored += b->data.storage.resourceStored[i];
		}
		int width = lang_text_draw(98, 2,
			c->xOffset + 34, c->yOffset + 40, FONT_NORMAL_BLACK);
		lang_text_draw_amount(8, 16, totalStored,
			c->xOffset + 34 + width, c->yOffset + 40, FONT_NORMAL_BLACK);

		width = lang_text_draw(98, 3,
			c->xOffset + 220, c->yOffset + 40, FONT_NORMAL_BLACK);
		lang_text_draw_amount(8, 16,
			b->data.storage.resourceStored[RESOURCE_NONE],
			c->xOffset + 220 + width, c->yOffset + 40, FONT_NORMAL_BLACK);

		int graphicId = image_group(GROUP_RESOURCE_ICONS);
		// wheat
		image_draw(graphicId + RESOURCE_WHEAT,
			c->xOffset + 34, c->yOffset + 68);
		width = text_draw_number(
			b->data.storage.resourceStored[RESOURCE_WHEAT], '@', " ",
			c->xOffset + 68, c->yOffset + 75, FONT_NORMAL_BLACK);
		lang_text_draw(23, RESOURCE_WHEAT,
			c->xOffset + 68 + width, c->yOffset + 75, FONT_NORMAL_BLACK);
		// vegetables
		image_draw(graphicId + RESOURCE_VEGETABLES,
			c->xOffset + 34, c->yOffset + 92);
		width = text_draw_number(
			b->data.storage.resourceStored[RESOURCE_VEGETABLES], '@', " ",
			c->xOffset + 68, c->yOffset + 99, FONT_NORMAL_BLACK);
		lang_text_draw(23, RESOURCE_VEGETABLES,
			c->xOffset + 68 + width, c->yOffset + 99, FONT_NORMAL_BLACK);
		// fruit
		image_draw(graphicId + RESOURCE_FRUIT,
			c->xOffset + 240, c->yOffset + 68);
		width = text_draw_number(
			b->data.storage.resourceStored[RESOURCE_FRUIT], '@', " ",
			c->xOffset + 274, c->yOffset + 75, FONT_NORMAL_BLACK);
		lang_text_draw(23, RESOURCE_FRUIT,
			c->xOffset + 274 + width, c->yOffset + 75, FONT_NORMAL_BLACK);
		// meat/fish
		image_draw(graphicId + RESOURCE_MEAT +
			resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
			c->xOffset + 240, c->yOffset + 92);
		width = text_draw_number(
			b->data.storage.resourceStored[RESOURCE_MEAT], '@', " ",
			c->xOffset + 274, c->yOffset + 99, FONT_NORMAL_BLACK);
		lang_text_draw(23, RESOURCE_MEAT,
			c->xOffset + 274 + width, c->yOffset + 99, FONT_NORMAL_BLACK);
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, 142);
}

void UI_BuildingInfo_drawGranaryForeground(BuildingInfoContext *c)
{
	button_border_draw(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		16 * (c->widthBlocks - 10), 20, focusButtonId == 1 ? 1 : 0);
	lang_text_draw_centered(98, 5,
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 30,
		16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_handleMouseGranary(const mouse *m, BuildingInfoContext *c)
{
	generic_buttons_handle_mouse(
		m, c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		gotoOrdersButtons, 1, &focusButtonId);
}

void UI_BuildingInfo_drawGranaryOrders(BuildingInfoContext *c)
{
	c->helpId = 3;
	outer_panel_draw(c->xOffset, 32, 29, 28);
	lang_text_draw_centered(98, 6,
		c->xOffset, 42, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	inner_panel_draw(c->xOffset + 16, 74, c->widthBlocks - 2, 21);
}

void UI_BuildingInfo_drawGranaryOrdersForeground(BuildingInfoContext *c)
{
	button_border_draw(
		c->xOffset + 80, 436, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 1 ? 1 : 0);
	const building_storage *storage = building_storage_get(building_get(c->buildingId)->storage_id);
	if (storage->empty_all) {
		lang_text_draw_centered(98, 8, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
		lang_text_draw_centered(98, 9, c->xOffset + 80, 416,
			16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_centered(98, 7, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
	}

	for (int i = 0; i < Data_CityInfo_Resource.numAvailableFoods; i++) {
		int resourceId = Data_CityInfo_Resource.availableFoods[i];
		int graphicId = image_group(GROUP_RESOURCE_ICONS) + resourceId +
			resource_image_offset(resourceId, RESOURCE_IMAGE_ICON);
		image_draw(graphicId, c->xOffset + 32, 78 + 22 * i);
		image_draw(graphicId, c->xOffset + 408, 78 + 22 * i);
		lang_text_draw(23, resourceId,
			c->xOffset + 72, 82 + 22 * i, FONT_NORMAL_WHITE);
		button_border_draw(c->xOffset + 180, 78 + 22 * i, 210, 22,
			resourceFocusButtonId == i + 1);
		
		int state = storage->resource_state[resourceId];
		if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
			lang_text_draw(99, 7, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_WHITE);
		} else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
			lang_text_draw(99, 8, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_RED);
		} else if (state == BUILDING_STORAGE_STATE_GETTING) {
			image_draw(image_group(GROUP_CONTEXT_ICONS) + 12,
				c->xOffset + 186, 81 + 22 * i);
			lang_text_draw(99, 10, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_WHITE);
		}
	}
}

void UI_BuildingInfo_handleMouseGranaryOrders(const mouse *m, BuildingInfoContext *c)
{
	buildingId = c->buildingId;
	if (generic_buttons_handle_mouse(m, c->xOffset + 180, 78,
		ordersResourceButtons, Data_CityInfo_Resource.numAvailableFoods,
		&resourceFocusButtonId)) {
		return;
	}
	generic_buttons_handle_mouse(m, c->xOffset + 80, 436,
		granaryOrderButtons, 1, &ordersFocusButtonId);
}

void UI_BuildingInfo_drawWarehouse(BuildingInfoContext *c)
{
	c->helpId = 4;
	window_building_play_sound(c, "wavs/warehouse.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(99, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	building *b = building_get(c->buildingId);
	if (!c->hasRoadAccess) {
		window_building_draw_description(c, 69, 25);
	} else {
		for (int r = 1; r < 16; r++) {
			int x, y;
			if (r <= 5) {
				x = c->xOffset + 20;
				y = c->yOffset + 24 * (r - 1) + 36;
			} else if (r <= 10) {
				x = c->xOffset + 170;
				y = c->yOffset + 24 * (r - 6) + 36;
			} else {
				x = c->xOffset + 320;
				y = c->yOffset + 24 * (r - 11) + 36;
			}
			int amount = building_warehouse_get_amount(b, r);
			int graphicId = image_group(GROUP_RESOURCE_ICONS) + r +
				resource_image_offset(r, RESOURCE_IMAGE_ICON);
			image_draw(graphicId, x, y);
			int width = text_draw_number(amount, '@', " ",
				x + 24, y + 7, FONT_SMALL_PLAIN);
			lang_text_draw(23, r,
				x + 24 + width, y + 7, FONT_SMALL_PLAIN);
		}
	}
	inner_panel_draw(c->xOffset + 16, c->yOffset + 168, c->widthBlocks - 2, 5);
	window_building_draw_employment(c, 173);
	// cartpusher state
	int cartpusher = b->figureId;
	if (cartpusher && figure_get(cartpusher)->state == FigureState_Alive) {
		int resource = figure_get(cartpusher)->resourceId;
		image_draw(image_group(GROUP_RESOURCE_ICONS) + resource +
			resource_image_offset(resource, RESOURCE_IMAGE_ICON),
			c->xOffset + 32, c->yOffset + 220);
		lang_text_draw_multiline(99, 17,
			c->xOffset + 64, c->yOffset + 223,
			16 * (c->widthBlocks - 6), FONT_SMALL_BLACK);
	} else if (b->numWorkers) {
		// cartpusher is waiting for orders
		lang_text_draw_multiline(99, 15,
			c->xOffset + 32, c->yOffset + 223,
			16 * (c->widthBlocks - 4), FONT_SMALL_BLACK);
	}

	if (c->warehouseSpaceText == 1) { // full
		lang_text_draw_multiline(99, 13,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 93,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else if (c->warehouseSpaceText == 2) {
		lang_text_draw_multiline(99, 14,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 93,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawWarehouseForeground(BuildingInfoContext *c)
{
	button_border_draw(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		16 * (c->widthBlocks - 10), 20, focusButtonId == 1 ? 1 : 0);
	lang_text_draw_centered(99, 2,
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 30,
		16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_handleMouseWarehouse(const mouse *m, BuildingInfoContext *c)
{
	generic_buttons_handle_mouse(
		m, c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		gotoOrdersButtons, 1, &focusButtonId);
}

void UI_BuildingInfo_drawWarehouseOrders(BuildingInfoContext *c)
{
	c->helpId = 4;
	outer_panel_draw(c->xOffset, 32, 29, 28);
	lang_text_draw_centered(99, 3,
		c->xOffset, 42, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	inner_panel_draw(c->xOffset + 16, 74, c->widthBlocks - 2, 21);
}

void UI_BuildingInfo_drawWarehouseOrdersForeground(BuildingInfoContext *c)
{
	button_border_draw(
		c->xOffset + 80, 436, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 1 ? 1 : 0);
	const building_storage *storage = building_storage_get(building_get(c->buildingId)->storage_id);
	if (storage->empty_all) {
		lang_text_draw_centered(99, 5, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
		lang_text_draw_centered(99, 6, c->xOffset + 80, 458,
			16 * (c->widthBlocks - 10), FONT_SMALL_PLAIN);
	} else {
		lang_text_draw_centered(99, 4, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);
	}

	// trade center
	button_border_draw(
		c->xOffset + 80, 414, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 2 ? 1 : 0);
	int isTradeCenter = c->buildingId == Data_CityInfo.buildingTradeCenterBuildingId;
	lang_text_draw_centered(99, isTradeCenter ? 11 : 12,
		c->xOffset + 80, 418, 16 * (c->widthBlocks - 10), FONT_NORMAL_BLACK);

	for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++) {
		int resourceId = Data_CityInfo_Resource.availableResources[i];
		int graphicId = image_group(GROUP_RESOURCE_ICONS) + resourceId +
			resource_image_offset(resourceId, RESOURCE_IMAGE_ICON);
		image_draw(graphicId, c->xOffset + 32, 78 + 22 * i);
		image_draw(graphicId, c->xOffset + 408, 78 + 22 * i);
		lang_text_draw(23, resourceId,
			c->xOffset + 72, 82 + 22 * i, FONT_NORMAL_WHITE);
		button_border_draw(c->xOffset + 180, 78 + 22 * i, 210, 22,
			resourceFocusButtonId == i + 1);
		
		int state = storage->resource_state[resourceId];
		if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
			lang_text_draw(99, 7, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_WHITE);
		} else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
			lang_text_draw(99, 8, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_RED);
		} else if (state == BUILDING_STORAGE_STATE_GETTING) {
			image_draw(image_group(GROUP_CONTEXT_ICONS) + 12,
				c->xOffset + 186, 81 + 22 * i);
			lang_text_draw(99, 9, c->xOffset + 230, 83 + 22 * i, FONT_NORMAL_WHITE);
		}
	}
}

void UI_BuildingInfo_handleMouseWarehouseOrders(const mouse *m, BuildingInfoContext *c)
{
	buildingId = c->buildingId;
	if (generic_buttons_handle_mouse(m, c->xOffset + 180, 78,
		ordersResourceButtons, Data_CityInfo_Resource.numAvailableResources,
		&resourceFocusButtonId)) {
		return;
	}
	generic_buttons_handle_mouse(m, c->xOffset + 80, 436,
		warehouseOrderButtons, 2, &ordersFocusButtonId);
}

static void toggleResourceState(int index, int param2)
{
    building *b = building_get(buildingId);
	int resourceId;
	if (b->type == BUILDING_WAREHOUSE) {
		resourceId = Data_CityInfo_Resource.availableResources[index-1];
	} else {
		resourceId = Data_CityInfo_Resource.availableFoods[index-1];
	}
	building_storage_cycle_resource_state(b->storage_id, resourceId);
	window_invalidate();
}

static void granaryOrders(int index, int param2)
{
	int storageId = building_get(buildingId)->storage_id;
	building_storage_toggle_empty_all(storageId);
	window_invalidate();
}

static void warehouseOrders(int index, int param2)
{
	if (index == 0) {
		int storageId = building_get(buildingId)->storage_id;
		building_storage_toggle_empty_all(storageId);
	} else if (index == 1) {
		Data_CityInfo.buildingTradeCenterBuildingId = buildingId;
	}
	window_invalidate();
}
