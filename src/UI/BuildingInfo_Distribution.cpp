#include "BuildingInfo.h"

#include "../Graphics.h"
#include "../Resource.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"

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

