#include "BuildingInfo.h"

#include "../Graphics.h"
#include "../Resource.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Scenario.h"
#include "../Data/Walker.h"

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
		if (b->data.market.food[0] || b->data.market.food[1] ||
			b->data.market.food[2] || b->data.market.food[3]) {
			// food stocks
			Graphics_drawImage(graphicId + Resource_Wheat,
				c->xOffset + 32, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.food[0], '@', " ",
				c->xOffset + 64, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Vegetables,
				c->xOffset + 142, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.food[1], '@', " ",
				c->xOffset + 174, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Fruit,
				c->xOffset + 252, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.food[2], '@', " ",
				c->xOffset + 284, c->yOffset + 70, Font_NormalBlack);
			Graphics_drawImage(graphicId + Resource_Meat +
				Resource_getGraphicIdOffset(Resource_Meat, 3),
				c->xOffset + 362, c->yOffset + 64);
			Widget_Text_drawNumber(b->data.market.food[3], '@', " ",
				c->xOffset + 394, c->yOffset + 70, Font_NormalBlack);
		} else {
			Widget_GameText_drawMultiline(97, 4,
				c->xOffset + 32, c->yOffset + 48,
				16 * (c->widthBlocks - 4), Font_NormalBlack);
		}
		// good stocks
		Graphics_drawImage(graphicId + Resource_Pottery,
			c->xOffset + 32, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.pottery, '@', " ",
			c->xOffset + 64, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Furniture,
			c->xOffset + 142, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.furniture, '@', " ",
			c->xOffset + 174, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Oil,
			c->xOffset + 252, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.oil, '@', " ",
			c->xOffset + 284, c->yOffset + 110, Font_NormalBlack);
		Graphics_drawImage(graphicId + Resource_Wine,
			c->xOffset + 362, c->yOffset + 104);
		Widget_Text_drawNumber(b->data.market.wine, '@', " ",
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
	int cartpusher = b->walkerId;
	if (cartpusher && Data_Walkers[cartpusher].state == WalkerState_Alive) {
		// TODO
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
