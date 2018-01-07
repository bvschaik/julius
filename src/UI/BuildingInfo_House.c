#include "BuildingInfo.h"

#include "core/calc.h"
#include "../Graphics.h"

#include "../Data/CityInfo.h"

#include "building/building.h"
#include "building/model.h"
#include "game/resource.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_access.h"

static void drawVacantLot(BuildingInfoContext *c)
{
	UI_BuildingInfo_drawFigureImagesLocal(c);
	outer_panel_draw(c->xOffset, c->yOffset,
		c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(128, 0, c->xOffset, c->yOffset + 8,
		16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_centered(13, 1, c->xOffset, c->yOffset + 16 * c->heightBlocks - 22,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
	UI_BuildingInfo_drawFigureList(c);

	int textId = 2;
    building *b = building_get(c->buildingId);
	if (map_closest_road_within_radius(b->x, b->y, 1, 2, 0, 0)) {
		textId = 1;
	}
	lang_text_draw_multiline(128, textId,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

static void drawPopulationInfo(BuildingInfoContext *c, int yOffset)
{
	building *b = building_get(c->buildingId);
	Graphics_drawImage(image_group(GROUP_CONTEXT_ICONS) + 13, c->xOffset + 34, yOffset + 4);
	int width = text_draw_number(b->housePopulation, '@', " ",
		c->xOffset + 50, yOffset + 14, FONT_SMALL_BLACK);
	width += lang_text_draw(127, 20, c->xOffset + 50 + width, yOffset + 14, FONT_SMALL_BLACK);
	
	if (b->housePopulationRoom < 0) {
		width += text_draw_number(-b->housePopulationRoom, '@', " ",
			c->xOffset + 50 + width, yOffset + 14, FONT_SMALL_BLACK);
		lang_text_draw(127, 21, c->xOffset + 50 + width, yOffset + 14, FONT_SMALL_BLACK);
	} else if (b->housePopulationRoom > 0) {
		width += lang_text_draw(127, 22, c->xOffset + 50 + width, yOffset + 14, FONT_SMALL_BLACK);
		text_draw_number(b->housePopulationRoom, '@', " ",
			c->xOffset + 50 + width, yOffset + 14, FONT_SMALL_BLACK);
	}
}

static void drawTaxInfo(BuildingInfoContext *c, int yOffset)
{
	building *b = building_get(c->buildingId);
	if (b->houseTaxCoverage) {
		int pct = calc_adjust_with_percentage(b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
		int width = lang_text_draw(127, 24, c->xOffset + 36, yOffset, FONT_SMALL_BLACK);
		width += lang_text_draw_amount(8, 0, pct,
			c->xOffset + 36 + width, yOffset, FONT_SMALL_BLACK);
		lang_text_draw(127, 25, c->xOffset + 36 + width, yOffset, FONT_SMALL_BLACK);
	} else {
		lang_text_draw(127, 23, c->xOffset + 36, yOffset, FONT_SMALL_BLACK);
	}
}

static void drawHappinessInfo(BuildingInfoContext *c, int yOffset)
{
	int happiness = building_get(c->buildingId)->sentiment.houseHappiness;
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
	lang_text_draw(127, textId, c->xOffset + 36, yOffset, FONT_SMALL_BLACK);
}

void UI_BuildingInfo_drawHouse(BuildingInfoContext *c)
{
	c->helpId = 56;
	PLAY_SOUND("wavs/housing.wav");
	building *b = building_get(c->buildingId);
	if (b->housePopulation <= 0) {
		drawVacantLot(c);
		return;
	}
	int level = b->type - 10;
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(29, level, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	inner_panel_draw(c->xOffset + 16, c->yOffset + 148, c->widthBlocks - 2, 10);
	
	drawPopulationInfo(c, c->yOffset + 154);
	drawTaxInfo(c, c->yOffset + 194);
	drawHappinessInfo(c, c->yOffset + 214);
	
	int resourceGraphic = image_group(GROUP_RESOURCE_ICONS);
	// food inventory
	if (model_get_house(b->subtype.houseLevel)->food_types) {
		// wheat
		Graphics_drawImage(resourceGraphic + RESOURCE_WHEAT,
			c->xOffset + 32, c->yOffset + 234);
		text_draw_number(b->data.house.inventory[INVENTORY_WHEAT], '@', " ",
			c->xOffset + 64, c->yOffset + 238, FONT_SMALL_BLACK);
		// vegetables
		Graphics_drawImage(resourceGraphic + RESOURCE_VEGETABLES,
			c->xOffset + 142, c->yOffset + 234);
		text_draw_number(b->data.house.inventory[INVENTORY_VEGETABLES], '@', " ",
			c->xOffset + 174, c->yOffset + 238, FONT_SMALL_BLACK);
		// fruit
		Graphics_drawImage(resourceGraphic + RESOURCE_FRUIT,
			c->xOffset + 252, c->yOffset + 234);
		text_draw_number(b->data.house.inventory[INVENTORY_FRUIT], '@', " ",
			c->xOffset + 284, c->yOffset + 238, FONT_SMALL_BLACK);
		// meat/fish
		Graphics_drawImage(resourceGraphic + RESOURCE_MEAT +
			resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
			c->xOffset + 362, c->yOffset + 234);
		text_draw_number(b->data.house.inventory[INVENTORY_MEAT], '@', " ",
			c->xOffset + 394, c->yOffset + 238, FONT_SMALL_BLACK);
	} else {
		// no food necessary
		lang_text_draw_multiline(127, 33, c->xOffset + 36, c->yOffset + 234,
			16 * (c->widthBlocks - 6), FONT_SMALL_BLACK);
	}
	// goods inventory
	// pottery
	Graphics_drawImage(resourceGraphic + RESOURCE_POTTERY,
		c->xOffset + 32, c->yOffset + 274);
	text_draw_number(b->data.house.inventory[INVENTORY_POTTERY], '@', " ",
		c->xOffset + 64, c->yOffset + 278, FONT_SMALL_BLACK);
	// furniture
	Graphics_drawImage(resourceGraphic + RESOURCE_FURNITURE,
		c->xOffset + 142, c->yOffset + 274);
	text_draw_number(b->data.house.inventory[INVENTORY_FURNITURE], '@', " ",
		c->xOffset + 174, c->yOffset + 278, FONT_SMALL_BLACK);
	// oil
	Graphics_drawImage(resourceGraphic + RESOURCE_OIL,
		c->xOffset + 252, c->yOffset + 274);
	text_draw_number(b->data.house.inventory[INVENTORY_OIL], '@', " ",
		c->xOffset + 284, c->yOffset + 278, FONT_SMALL_BLACK);
	// wine
	Graphics_drawImage(resourceGraphic + RESOURCE_WINE,
		c->xOffset + 362, c->yOffset + 274);
	text_draw_number(b->data.house.inventory[INVENTORY_WINE], '@', " ",
		c->xOffset + 394, c->yOffset + 278, FONT_SMALL_BLACK);
	
	if (b->data.house.evolveTextId == 62) {
		int width = lang_text_draw(127, 40 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 60, FONT_NORMAL_BLACK);
		width += lang_text_draw_colored(41, building_get(c->worstDesirabilityBuildingId)->type,
			c->xOffset + 32 + width, c->yOffset + 60, FONT_NORMAL_PLAIN, COLOR_RED);
		text_draw((uint8_t*)")", c->xOffset + 32 + width, c->yOffset + 60, FONT_NORMAL_BLACK, 0);
		lang_text_draw_multiline(127, 41 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 76, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_multiline(127, 40 + b->data.house.evolveTextId,
			c->xOffset + 32, c->yOffset + 70, 16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_houseDetermineWorstDesirabilityBuilding(BuildingInfoContext *c)
{
	int lowestDesirability = 0;
	int lowestBuildingId = 0;
    building *cb = building_get(c->buildingId);
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(cb->x, cb->y, 1, 6, &x_min, &y_min, &x_max, &y_max);

	for (int y = y_min; y <= y_max; y++) {
		for (int x = x_min; x <= x_max; x++) {
			int buildingId = map_building_at(map_grid_offset(x, y));
			if (buildingId <= 0) {
				continue;
			}
			building *b = building_get(buildingId);
			if (b->state != BUILDING_STATE_IN_USE || buildingId == c->buildingId) {
				continue;
			}
			if (!b->houseSize || b->type < cb->type) {
				int des = model_get_building(b->type)->desirability_value;
				if (des < 0) {
					// simplified desirability calculation
					int stepSize = model_get_building(b->type)->desirability_step_size;
					int range = model_get_building(b->type)->desirability_range;
					int dist = calc_maximum_distance(x, y, cb->x, cb->y);
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
