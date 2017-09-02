#include "BuildingInfo.h"

#include "core/calc.h"
#include "../Graphics.h"
#include "../Resource.h"
#include "../Sound.h"
#include "../Terrain.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Grid.h"
#include "../Data/Settings.h"

#include "building/model.h"
#include "graphics/image.h"

static void drawVacantLot(BuildingInfoContext *c)
{
	UI_BuildingInfo_drawFigureImagesLocal(c);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset,
		c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(128, 0, c->xOffset, c->yOffset + 8,
		16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawCentered(13, 1, c->xOffset, c->yOffset + 16 * c->heightBlocks - 22,
		16 * c->widthBlocks, Font_NormalBlack);
	UI_BuildingInfo_drawFigureList(c);

	int textId = 2;
	if (Terrain_getClosestRoadWithinRadius(
		Data_Buildings[c->buildingId].x,
		Data_Buildings[c->buildingId].y, 1, 2, 0, 0)) {
		textId = 1;
	}
	Widget_GameText_drawMultiline(128, textId,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

static void drawPopulationInfo(BuildingInfoContext *c, int yOffset)
{
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	Graphics_drawImage(image_group(ID_Graphic_ContextIcons) + 13, c->xOffset + 34, yOffset + 4);
	int width = Widget_Text_drawNumber(b->housePopulation, '@', " ",
		c->xOffset + 50, yOffset + 14, Font_SmallBlack);
	width += Widget_GameText_draw(127, 20, c->xOffset + 50 + width, yOffset + 14, Font_SmallBlack);
	
	if (b->housePopulationRoom < 0) {
		width += Widget_Text_drawNumber(-b->housePopulationRoom, '@', " ",
			c->xOffset + 50 + width, yOffset + 14, Font_SmallBlack);
		Widget_GameText_draw(127, 21, c->xOffset + 50 + width, yOffset + 14, Font_SmallBlack);
	} else if (b->housePopulationRoom > 0) {
		width += Widget_GameText_draw(127, 22, c->xOffset + 50 + width, yOffset + 14, Font_SmallBlack);
		Widget_Text_drawNumber(b->housePopulationRoom, '@', " ",
			c->xOffset + 50 + width, yOffset + 14, Font_SmallBlack);
	}
}

static void drawTaxInfo(BuildingInfoContext *c, int yOffset)
{
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	if (b->houseTaxCoverage) {
		int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
		int width = Widget_GameText_draw(127, 24, c->xOffset + 36, yOffset, Font_SmallBlack);
		width += Widget_GameText_drawNumberWithDescription(8, 0, pct,
			c->xOffset + 36 + width, yOffset, Font_SmallBlack);
		Widget_GameText_draw(127, 25, c->xOffset + 36 + width, yOffset, Font_SmallBlack);
	} else {
		Widget_GameText_draw(127, 23, c->xOffset + 36, yOffset, Font_SmallBlack);
	}
}

static void drawHappinessInfo(BuildingInfoContext *c, int yOffset)
{
	int happiness = Data_Buildings[c->buildingId].sentiment.houseHappiness;
	int textId;
	if (happiness >= 50) {
		textId = 26;
	} else if (happiness >= 40) {
		textId = 27;
	} else if (happiness >= 30) {
		textId = 28;
	} else if (happiness >= 20) {
		textId = 29;
	} else if (happiness >= 10) {
		textId = 30;
	} else if (happiness >= 1) {
		textId = 31;
	} else {
		textId = 32;
	}
	Widget_GameText_draw(127, textId, c->xOffset + 36, yOffset, Font_SmallBlack);
}

void UI_BuildingInfo_drawHouse(BuildingInfoContext *c)
{
	c->helpId = 56;
	PLAY_SOUND("wavs/housing.wav");
	struct Data_Building *b = &Data_Buildings[c->buildingId];
	if (b->housePopulation <= 0) {
		drawVacantLot(c);
		return;
	}
	int level = b->type - 10;
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(29, level, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 148, c->widthBlocks - 2, 10);
	
	drawPopulationInfo(c, c->yOffset + 154);
	drawTaxInfo(c, c->yOffset + 194);
	drawHappinessInfo(c, c->yOffset + 214);
	
	int resourceGraphic = image_group(ID_Graphic_ResourceIcons);
	// food inventory
	if (model_get_house(b->subtype.houseLevel)->food_types) {
		// wheat
		Graphics_drawImage(resourceGraphic + Resource_Wheat,
			c->xOffset + 32, c->yOffset + 234);
		Widget_Text_drawNumber(b->data.house.inventory[Inventory_Wheat], '@', " ",
			c->xOffset + 64, c->yOffset + 238, Font_SmallBlack);
		// vegetables
		Graphics_drawImage(resourceGraphic + Resource_Vegetables,
			c->xOffset + 142, c->yOffset + 234);
		Widget_Text_drawNumber(b->data.house.inventory[Inventory_Vegetables], '@', " ",
			c->xOffset + 174, c->yOffset + 238, Font_SmallBlack);
		// fruit
		Graphics_drawImage(resourceGraphic + Resource_Fruit,
			c->xOffset + 252, c->yOffset + 234);
		Widget_Text_drawNumber(b->data.house.inventory[Inventory_Fruit], '@', " ",
			c->xOffset + 284, c->yOffset + 238, Font_SmallBlack);
		// meat/fish
		Graphics_drawImage(resourceGraphic + Resource_Meat +
			Resource_getGraphicIdOffset(Resource_Meat, 3),
			c->xOffset + 362, c->yOffset + 234);
		Widget_Text_drawNumber(b->data.house.inventory[Inventory_Meat], '@', " ",
			c->xOffset + 394, c->yOffset + 238, Font_SmallBlack);
	} else {
		// no food necessary
		Widget_GameText_drawMultiline(127, 33, c->xOffset + 36, c->yOffset + 234,
			16 * (c->widthBlocks - 6), Font_SmallBlack);
	}
	// goods inventory
	// pottery
	Graphics_drawImage(resourceGraphic + Resource_Pottery,
		c->xOffset + 32, c->yOffset + 274);
	Widget_Text_drawNumber(b->data.house.inventory[Inventory_Pottery], '@', " ",
		c->xOffset + 64, c->yOffset + 278, Font_SmallBlack);
	// furniture
	Graphics_drawImage(resourceGraphic + Resource_Furniture,
		c->xOffset + 142, c->yOffset + 274);
	Widget_Text_drawNumber(b->data.house.inventory[Inventory_Furniture], '@', " ",
		c->xOffset + 174, c->yOffset + 278, Font_SmallBlack);
	// oil
	Graphics_drawImage(resourceGraphic + Resource_Oil,
		c->xOffset + 252, c->yOffset + 274);
	Widget_Text_drawNumber(b->data.house.inventory[Inventory_Oil], '@', " ",
		c->xOffset + 284, c->yOffset + 278, Font_SmallBlack);
	// wine
	Graphics_drawImage(resourceGraphic + Resource_Wine,
		c->xOffset + 362, c->yOffset + 274);
	Widget_Text_drawNumber(b->data.house.inventory[Inventory_Wine], '@', " ",
		c->xOffset + 394, c->yOffset + 278, Font_SmallBlack);
	
	if (b->data.house.evolveTextId == 62) {
		int width = Widget_GameText_draw(127, 40 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 60, Font_NormalBlack);
		width += Widget_GameText_drawColored(41, Data_Buildings[c->worstDesirabilityBuildingId].type,
			c->xOffset + 32 + width, c->yOffset + 60, Font_NormalPlain, COLOR_RED);
		Widget_Text_draw((uint8_t*)")", c->xOffset + 32 + width, c->yOffset + 60, Font_NormalBlack, 0);
		Widget_GameText_drawMultiline(127, 41 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 76, 16 * (c->widthBlocks - 4), Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(127, 40 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 70, 16 * (c->widthBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_houseDetermineWorstDesirabilityBuilding(BuildingInfoContext *c)
{
	int lowestDesirability = 0;
	int lowestBuildingId = 0;
	int bx = Data_Buildings[c->buildingId].x;
	int by = Data_Buildings[c->buildingId].y;
	int xMin = bx - 6;
	int yMin = by - 6;
	int xMax = bx + 6;
	int yMax = by + 6;
	Bound2ToMap(xMin, yMin, xMax, yMax);

	for (int y = yMin; y <= yMax; y++) {
		for (int x = xMin; x <= xMax; x++) {
			int buildingId = Data_Grid_buildingIds[GridOffset(x, y)];
			if (buildingId <= 0) {
				continue;
			}
			struct Data_Building *b = &Data_Buildings[buildingId];
			if (!BuildingIsInUse(buildingId) || buildingId == c->buildingId) {
				continue;
			}
			if (!b->houseSize || b->type < Data_Buildings[c->buildingId].type) {
				int des = model_get_building(b->type)->desirability_value;
				if (des < 0) {
					// simplified desirability calculation
					int stepSize = model_get_building(b->type)->desirability_step_size;
					int range = model_get_building(b->type)->desirability_range;
					int dist = calc_maximum_distance(x, y,
						Data_Buildings[c->buildingId].x, Data_Buildings[c->buildingId].y);
					if (dist <= range) {
						while (--dist > 1) {
							des += stepSize;
						}
						if (des < lowestDesirability) {
							lowestDesirability = des;
							lowestBuildingId = buildingId;
						}
					}
				}
			}
		}
	}
	c->worstDesirabilityBuildingId = lowestBuildingId;
}
