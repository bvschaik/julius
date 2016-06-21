#include "BuildingInfo.h"

#include "Window.h"

#include "../Graphics.h"
#include "../Resource.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Figure.h"
#include "../Data/Scenario.h"

static void toggleResourceState(int param1, int param2);
static void granaryOrders(int index, int param2);
static void warehouseOrders(int index, int param2);

static CustomButton gotoOrdersButtons[] = {
	{0, 0, 304, 20, CustomButton_Immediate, UI_BuildingInfo_showStorageOrders, Widget_Button_doNothing, 0, 0}
};

static CustomButton ordersResourceButtons[] = {
	{0, 0, 210, 22, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 1, 0},
	{0, 22, 210, 44, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 2, 0},
	{0, 44, 210, 66, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 3, 0},
	{0, 66, 210, 88, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 4, 0},
	{0, 88, 210, 110, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 5, 0},
	{0, 110, 210, 132, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 6, 0},
	{0, 132, 210, 154, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 7, 0},
	{0, 154, 210, 176, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 8, 0},
	{0, 176, 210, 198, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 9, 0},
	{0, 198, 210, 220, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 10, 0},
	{0, 220, 210, 242, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 11, 0},
	{0, 242, 210, 264, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 12, 0},                                                   
	{0, 264, 210, 286, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 13, 0},
	{0, 286, 210, 308, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 14, 0},
	{0, 308, 210, 330, CustomButton_Immediate, toggleResourceState, Widget_Button_doNothing, 15, 0},
};

static CustomButton granaryOrderButtons[] = {
	{0, 0, 304, 20, CustomButton_Immediate, granaryOrders, Widget_Button_doNothing, 0, 0},
};

static CustomButton warehouseOrderButtons[] = {
	{0, 0, 304, 20, CustomButton_Immediate, warehouseOrders, Widget_Button_doNothing, 0, 0},
	{0, -22, 304, 20, CustomButton_Immediate, warehouseOrders, Widget_Button_doNothing, 1, 0},
};

static int focusButtonId = 0;
static int ordersFocusButtonId = 0;
static int resourceFocusButtonId = 0;
static int buildingId;

void UI_BuildingInfo_drawMarket(BuildingInfoContext *c)
{
	c->helpId = 2;
	PLAY_SOUND("wavs/market.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(97, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(97, 2);
	} else {
		int graphicId = GraphicId(ID_Graphic_ResourceIcons);
		if (b->data.market.inventory[Inventory_Wheat] || b->data.market.inventory[Inventory_Vegetables] ||
			b->data.market.inventory[Inventory_Fruit] || b->data.market.inventory[Inventory_Meat]) {
			// food stocks
			Graphics_drawImage(graphicId + Resource_Wheat,
				c->xOffset + 32, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.inventory[Inventory_Wheat], '@', " ",
				c->xOffset + 64, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Vegetables,
				c->xOffset + 142, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.inventory[Inventory_Vegetables], '@', " ",
				c->xOffset + 174, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Fruit,
				c->xOffset + 252, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.inventory[Inventory_Fruit], '@', " ",
				c->xOffset + 284, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Meat +
				Resource_getGraphicIdOffset(Resource_Meat, 3),
				c->xOffset + 362, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.inventory[Inventory_Meat], '@', " ",
				c->xOffset + 394, c->yOffset + 70, Font_NormalBlack);
		} else {
			Widget_GameText_drawMultiline(97, 4,
				c->xOffset + 32, c->yOffset + 48,
				16 * (c->widthBlocks - 4), Font_NormalBlack);
		}
		// good stocks
		Graphics_drawImage(graphicId + Resource_Pottery,
			c->xOffset + 32, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.inventory[Inventory_Pottery], '@', " ",
			c->xOffset + 64, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Furniture,
			c->xOffset + 142, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.inventory[Inventory_Furniture], '@', " ",
			c->xOffset + 174, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Oil,
			c->xOffset + 252, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.inventory[Inventory_Oil], '@', " ",
			c->xOffset + 284, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Wine,
			c->xOffset + 362, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.inventory[Inventory_Wine], '@', " ",
			c->xOffset + 394, c->yOffset + 110, Font_NormalBlack);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawGranary(BuildingInfoContext *c)
{
	c->helpId = 3;
	PLAY_SOUND("wavs/granary.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(98, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	if (!c->hasRoadAccess) {
		DRAW_DESC_AT(40, 69, 25);
	} else if (Data_Scenario.romeSuppliesWheat) {
		DRAW_DESC_AT(40, 98, 4);
	} else {
		int totalStored = 0;
		for (int i = Resource_Wheat; i <= Resource_Meat; i++) {
			totalStored += b->data.storage.resourceStored[i];
		}
		int width = Widget_GameText_draw(98, 2,
			c->xOffset + 34, c->yOffset + 40, Font_NormalBlack);
		Widget_GameText_drawNumberWithDescription(8, 16, totalStored,
			c->xOffset + 34 + width, c->yOffset + 40, Font_NormalBlack);

		width = Widget_GameText_draw(98, 3,
			c->xOffset + 220, c->yOffset + 40, Font_NormalBlack);
		Widget_GameText_drawNumberWithDescription(8, 16,
			b->data.storage.resourceStored[Resource_None],
			c->xOffset + 220 + width, c->yOffset + 40, Font_NormalBlack);

		int graphicId = GraphicId(ID_Graphic_ResourceIcons);
		// wheat
		Graphics_drawImage(graphicId + Resource_Wheat,
			c->xOffset + 34, c->yOffset + 68);
		width = Widget_Text_drawNumber(
			b->data.storage.resourceStored[Resource_Wheat], '@', " ",
			c->xOffset + 68, c->yOffset + 75, Font_NormalBlack);
		Widget_GameText_draw(23, Resource_Wheat,
			c->xOffset + 68 + width, c->yOffset + 75, Font_NormalBlack);
		// vegetables
		Graphics_drawImage(graphicId + Resource_Vegetables,
			c->xOffset + 34, c->yOffset + 92);
		width = Widget_Text_drawNumber(
			b->data.storage.resourceStored[Resource_Vegetables], '@', " ",
			c->xOffset + 68, c->yOffset + 99, Font_NormalBlack);
		Widget_GameText_draw(23, Resource_Vegetables,
			c->xOffset + 68 + width, c->yOffset + 99, Font_NormalBlack);
		// fruit
		Graphics_drawImage(graphicId + Resource_Fruit,
			c->xOffset + 240, c->yOffset + 68);
		width = Widget_Text_drawNumber(
			b->data.storage.resourceStored[Resource_Fruit], '@', " ",
			c->xOffset + 274, c->yOffset + 75, Font_NormalBlack);
		Widget_GameText_draw(23, Resource_Fruit,
			c->xOffset + 274 + width, c->yOffset + 75, Font_NormalBlack);
		// meat/fish
		Graphics_drawImage(graphicId + Resource_Meat +
			Resource_getGraphicIdOffset(Resource_Meat, 3),
			c->xOffset + 240, c->yOffset + 92);
		width = Widget_Text_drawNumber(
			b->data.storage.resourceStored[Resource_Meat], '@', " ",
			c->xOffset + 274, c->yOffset + 99, Font_NormalBlack);
		Widget_GameText_draw(23, Resource_Meat,
			c->xOffset + 274 + width, c->yOffset + 99, Font_NormalBlack);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawGranaryForeground(BuildingInfoContext *c)
{
	Widget_Panel_drawButtonBorder(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		16 * (c->widthBlocks - 10), 20, focusButtonId == 1 ? 1 : 0);
	Widget_GameText_drawCentered(98, 5,
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 30,
		16 * (c->widthBlocks - 10), Font_NormalBlack);
}

void UI_BuildingInfo_handleMouseGranary(BuildingInfoContext *c)
{
	Widget_Button_handleCustomButtons(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		gotoOrdersButtons, 1, &focusButtonId);
}

void UI_BuildingInfo_drawGranaryOrders(BuildingInfoContext *c)
{
	c->helpId = 3;
	Widget_Panel_drawOuterPanel(c->xOffset, 32, 29, 28);
	Widget_GameText_drawCentered(98, 6,
		c->xOffset, 42, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_Panel_drawInnerPanel(c->xOffset + 16, 74, c->widthBlocks - 2, 21);
}

void UI_BuildingInfo_drawGranaryOrdersForeground(BuildingInfoContext *c)
{
	Widget_Panel_drawButtonBorder(
		c->xOffset + 80, 436, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 1 ? 1 : 0);
	int storageId = Data_Buildings[c->buildingId].storageId;
	if (Data_Building_Storages[storageId].emptyAll) {
		Widget_GameText_drawCentered(98, 8, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), Font_NormalBlack);
		Widget_GameText_drawCentered(98, 9, c->xOffset + 80, 416,
			16 * (c->widthBlocks - 10), Font_NormalBlack);
	} else {
		Widget_GameText_drawCentered(98, 7, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), Font_NormalBlack);
	}

	for (int i = 0; i < Data_CityInfo_Resource.numAvailableFoods; i++) {
		int resourceId = Data_CityInfo_Resource.availableFoods[i];
		int graphicId = GraphicId(ID_Graphic_ResourceIcons) + resourceId +
			Resource_getGraphicIdOffset(resourceId, 3);
		Graphics_drawImage(graphicId, c->xOffset + 32, 78 + 22 * i);
		Graphics_drawImage(graphicId, c->xOffset + 408, 78 + 22 * i);
		Widget_GameText_draw(23, resourceId,
			c->xOffset + 72, 82 + 22 * i, Font_NormalWhite);
		Widget_Panel_drawButtonBorder(c->xOffset + 180, 78 + 22 * i, 210, 22,
			resourceFocusButtonId == i + 1);
		
		int state = Data_Building_Storages[storageId].resourceState[resourceId];
		if (state == BuildingStorageState_Accepting) {
			Widget_GameText_draw(99, 7, c->xOffset + 230, 83 + 22 * i, Font_NormalWhite);
		} else if (state == BuildingStorageState_NotAccepting) {
			Widget_GameText_draw(99, 8, c->xOffset + 230, 83 + 22 * i, Font_NormalRed);
		} else if (state == BuildingStorageState_Getting) {
			Graphics_drawImage(GraphicId(ID_Graphic_ContextIcons) + 12,
				c->xOffset + 186, 81 + 22 * i);
			Widget_GameText_draw(99, 10, c->xOffset + 230, 83 + 22 * i, Font_NormalWhite);
		}
	}
}

void UI_BuildingInfo_handleMouseGranaryOrders(BuildingInfoContext *c)
{
	buildingId = c->buildingId;
	if (Widget_Button_handleCustomButtons(c->xOffset + 180, 78,
		ordersResourceButtons, Data_CityInfo_Resource.numAvailableFoods,
		&resourceFocusButtonId)) {
		return;
	}
	Widget_Button_handleCustomButtons(c->xOffset + 80, 436,
		granaryOrderButtons, 1, &ordersFocusButtonId);
}

void UI_BuildingInfo_drawWarehouse(BuildingInfoContext *c)
{
	c->helpId = 4;
	PLAY_SOUND("wavs/warehouse.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(99, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
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
			int amount = Resource_getAmountStoredInWarehouse(c->buildingId, r);
			int graphicId = GraphicId(ID_Graphic_ResourceIcons) + r +
				Resource_getGraphicIdOffset(r, 3);
			Graphics_drawImage(graphicId, x, y);
			int width = Widget_Text_drawNumber(amount, '@', " ",
				x + 24, y + 7, Font_SmallPlain);
			Widget_GameText_draw(23, r,
				x + 24 + width, y + 7, Font_SmallPlain);
		}
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 168, c->widthBlocks - 2, 5);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 173);
	// cartpusher state
	int cartpusher = b->figureId;
	if (cartpusher && Data_Walkers[cartpusher].state == FigureState_Alive) {
		int resource = Data_Walkers[cartpusher].resourceId;
		Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + resource +
			Resource_getGraphicIdOffset(resource, 3),
			c->xOffset + 32, c->yOffset + 220);
		Widget_GameText_drawMultiline(99, 17,
			c->xOffset + 64, c->yOffset + 223,
			16 * (c->widthBlocks - 6), Font_SmallBlack);
	} else if (b->numWorkers) {
		// cartpusher is waiting for orders
		Widget_GameText_drawMultiline(99, 15,
			c->xOffset + 32, c->yOffset + 223,
			16 * (c->widthBlocks - 4), Font_SmallBlack);
	}

	if (c->warehouseSpaceText == 1) { // full
		Widget_GameText_drawMultiline(99, 13,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 93,
			16 * (c->widthBlocks - 4), Font_NormalBlack);
	} else if (c->warehouseSpaceText == 2) {
		Widget_GameText_drawMultiline(99, 14,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 93,
			16 * (c->widthBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_drawWarehouseForeground(BuildingInfoContext *c)
{
	Widget_Panel_drawButtonBorder(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		16 * (c->widthBlocks - 10), 20, focusButtonId == 1 ? 1 : 0);
	Widget_GameText_drawCentered(99, 2,
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 30,
		16 * (c->widthBlocks - 10), Font_NormalBlack);
}

void UI_BuildingInfo_handleMouseWarehouse(BuildingInfoContext *c)
{
	Widget_Button_handleCustomButtons(
		c->xOffset + 80, c->yOffset + 16 * c->heightBlocks - 34,
		gotoOrdersButtons, 1, &focusButtonId);
}

void UI_BuildingInfo_drawWarehouseOrders(BuildingInfoContext *c)
{
	c->helpId = 4;
	Widget_Panel_drawOuterPanel(c->xOffset, 32, 29, 28);
	Widget_GameText_drawCentered(99, 3,
		c->xOffset, 42, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_Panel_drawInnerPanel(c->xOffset + 16, 74, c->widthBlocks - 2, 21);
}

void UI_BuildingInfo_drawWarehouseOrdersForeground(BuildingInfoContext *c)
{
	Widget_Panel_drawButtonBorder(
		c->xOffset + 80, 436, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 1 ? 1 : 0);
	int storageId = Data_Buildings[c->buildingId].storageId;
	if (Data_Building_Storages[storageId].emptyAll) {
		Widget_GameText_drawCentered(99, 5, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), Font_NormalBlack);
		Widget_GameText_drawCentered(99, 6, c->xOffset + 80, 458,
			16 * (c->widthBlocks - 10), Font_SmallPlain);
	} else {
		Widget_GameText_drawCentered(99, 4, c->xOffset + 80, 440,
			16 * (c->widthBlocks - 10), Font_NormalBlack);
	}

	// trade center
	Widget_Panel_drawButtonBorder(
		c->xOffset + 80, 414, 16 * (c->widthBlocks - 10), 20,
		ordersFocusButtonId == 2 ? 1 : 0);
	int isTradeCenter = c->buildingId == Data_CityInfo.buildingTradeCenterBuildingId;
	Widget_GameText_drawCentered(99, isTradeCenter ? 11 : 12,
		c->xOffset + 80, 418, 16 * (c->widthBlocks - 10), Font_NormalBlack);

	for (int i = 0; i < Data_CityInfo_Resource.numAvailableResources; i++) {
		int resourceId = Data_CityInfo_Resource.availableResources[i];
		int graphicId = GraphicId(ID_Graphic_ResourceIcons) + resourceId +
			Resource_getGraphicIdOffset(resourceId, 3);
		Graphics_drawImage(graphicId, c->xOffset + 32, 78 + 22 * i);
		Graphics_drawImage(graphicId, c->xOffset + 408, 78 + 22 * i);
		Widget_GameText_draw(23, resourceId,
			c->xOffset + 72, 82 + 22 * i, Font_NormalWhite);
		Widget_Panel_drawButtonBorder(c->xOffset + 180, 78 + 22 * i, 210, 22,
			resourceFocusButtonId == i + 1);
		
		int state = Data_Building_Storages[storageId].resourceState[resourceId];
		if (state == BuildingStorageState_Accepting) {
			Widget_GameText_draw(99, 7, c->xOffset + 230, 83 + 22 * i, Font_NormalWhite);
		} else if (state == BuildingStorageState_NotAccepting) {
			Widget_GameText_draw(99, 8, c->xOffset + 230, 83 + 22 * i, Font_NormalRed);
		} else if (state == BuildingStorageState_Getting) {
			Graphics_drawImage(GraphicId(ID_Graphic_ContextIcons) + 12,
				c->xOffset + 186, 81 + 22 * i);
			Widget_GameText_draw(99, 9, c->xOffset + 230, 83 + 22 * i, Font_NormalWhite);
		}
	}
}

void UI_BuildingInfo_handleMouseWarehouseOrders(BuildingInfoContext *c)
{
	buildingId = c->buildingId;
	if (Widget_Button_handleCustomButtons(c->xOffset + 180, 78,
		ordersResourceButtons, Data_CityInfo_Resource.numAvailableResources,
		&resourceFocusButtonId)) {
		return;
	}
	Widget_Button_handleCustomButtons(c->xOffset + 80, 436,
		warehouseOrderButtons, 2, &ordersFocusButtonId);
}

static void toggleResourceState(int index, int param2)
{
	int storageId = Data_Buildings[buildingId].storageId;
	int resourceId;
	if (Data_Buildings[buildingId].type == Building_Warehouse) {
		resourceId = Data_CityInfo_Resource.availableResources[index-1];
	} else {
		resourceId = Data_CityInfo_Resource.availableFoods[index-1];
	}
	int state = Data_Building_Storages[storageId].resourceState[resourceId];
	if (state == BuildingStorageState_Accepting) {
		state = BuildingStorageState_NotAccepting;
	} else if (state == BuildingStorageState_NotAccepting) {
		state = BuildingStorageState_Getting;
	} else if (state == BuildingStorageState_Getting) {
		state = BuildingStorageState_Accepting;
	}
	Data_Building_Storages[storageId].resourceState[resourceId] = state;
	UI_Window_requestRefresh();
}

static void granaryOrders(int index, int param2)
{
	int storageId = Data_Buildings[buildingId].storageId;
	if (Data_Building_Storages[storageId].emptyAll) {
		Data_Building_Storages[storageId].emptyAll = 0;
	} else {
		Data_Building_Storages[storageId].emptyAll = 1;
	}
	UI_Window_requestRefresh();
}

static void warehouseOrders(int index, int param2)
{
	if (index == 0) {
		int storageId = Data_Buildings[buildingId].storageId;
		if (Data_Building_Storages[storageId].emptyAll) {
			Data_Building_Storages[storageId].emptyAll = 0;
		} else {
			Data_Building_Storages[storageId].emptyAll = 1;
		}
	} else if (index == 1) {
		Data_CityInfo.buildingTradeCenterBuildingId = buildingId;
	}
	UI_Window_requestRefresh();
}
