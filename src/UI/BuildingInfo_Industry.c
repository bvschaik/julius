#include "BuildingInfo.h"
#include "../Data/CityInfo.h"

#include "building/building.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void drawFarm(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId)
{
	c->helpId = helpId;
	window_building_play_sound(c, soundFile);

	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	image_draw(image_group(GROUP_RESOURCE_ICONS) + resourceId,
		c->xOffset + 10, c->yOffset + 10);
	lang_text_draw_centered(groupId, 0, c->xOffset, c->yOffset + 10,
		16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
	int pctGrown = calc_percentage(b->data.industry.progress, 200);
	int width = lang_text_draw(groupId, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
	width += text_draw_percentage(pctGrown,
		c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
	lang_text_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

	if (!c->hasRoadAccess) {
		window_building_draw_description_at(c, 70, 69, 25);
	} else if (Data_CityInfo.resourceIndustryMothballed[resourceId]) {
		window_building_draw_description_at(c, 70, groupId, 4);
	} else if (b->data.industry.curseDaysLeft > 4) {
		window_building_draw_description_at(c, 70, groupId, 11);
	} else if (b->numWorkers <= 0) {
		window_building_draw_description_at(c, 70, groupId, 5);
	} else if (c->workerPercentage >= 100) {
		window_building_draw_description_at(c, 70, groupId, 6);
	} else if (c->workerPercentage >= 75) {
		window_building_draw_description_at(c, 70, groupId, 7);
	} else if (c->workerPercentage >= 50) {
		window_building_draw_description_at(c, 70, groupId, 8);
	} else if (c->workerPercentage >= 25) {
		window_building_draw_description_at(c, 70, groupId, 9);
	} else {
		window_building_draw_description_at(c, 70, groupId, 10);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, c->yOffset + 142);
	lang_text_draw_multiline(groupId, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawWheatFarm(BuildingInfoContext *c)
{
	drawFarm(c, 89, "wavs/wheat_farm.wav", 112, RESOURCE_WHEAT);
}

void UI_BuildingInfo_drawVegetableFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/veg_farm.wav", 113, RESOURCE_VEGETABLES);
}

void UI_BuildingInfo_drawFruitFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/figs_farm.wav", 114, RESOURCE_FRUIT);
}

void UI_BuildingInfo_drawOliveFarm(BuildingInfoContext *c)
{
	drawFarm(c, 91, "wavs/olives_farm.wav", 115, RESOURCE_OLIVES);
}

void UI_BuildingInfo_drawVinesFarm(BuildingInfoContext *c)
{
	drawFarm(c, 91, "wavs/vines_farm.wav", 116, RESOURCE_VINES);
}

void UI_BuildingInfo_drawPigFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/meat_farm.wav", 117, RESOURCE_MEAT);
}

static void drawRawMaterial(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId)
{
	c->helpId = helpId;
	window_building_play_sound(c, soundFile);

	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	image_draw(image_group(GROUP_RESOURCE_ICONS) + resourceId,
		c->xOffset + 10, c->yOffset + 10);
	lang_text_draw_centered(groupId, 0, c->xOffset, c->yOffset + 10,
		16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
	int pctDone = calc_percentage(b->data.industry.progress, 200);
	int width = lang_text_draw(groupId, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
	width += text_draw_percentage(pctDone,
		c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
	lang_text_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

	if (!c->hasRoadAccess) {
		window_building_draw_description_at(c, 70, 69, 25);
	} else if (Data_CityInfo.resourceIndustryMothballed[resourceId]) {
		window_building_draw_description_at(c, 70, groupId, 4);
	} else if (b->numWorkers <= 0) {
		window_building_draw_description_at(c, 70, groupId, 5);
	} else if (c->workerPercentage >= 100) {
		window_building_draw_description_at(c, 70, groupId, 6);
	} else if (c->workerPercentage >= 75) {
		window_building_draw_description_at(c, 70, groupId, 7);
	} else if (c->workerPercentage >= 50) {
		window_building_draw_description_at(c, 70, groupId, 8);
	} else if (c->workerPercentage >= 25) {
		window_building_draw_description_at(c, 70, groupId, 9);
	} else {
		window_building_draw_description_at(c, 70, groupId, 10);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, c->yOffset + 142);
	lang_text_draw_multiline(groupId, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawMarbleQuarry(BuildingInfoContext *c)
{
	drawRawMaterial(c, 95, "wavs/quarry.wav", 118, RESOURCE_MARBLE);
}

void UI_BuildingInfo_drawIronMine(BuildingInfoContext *c)
{
	drawRawMaterial(c, 93, "wavs/mine.wav", 119, RESOURCE_IRON);
}

void UI_BuildingInfo_drawTimberYard(BuildingInfoContext *c)
{
	drawRawMaterial(c, 94, "wavs/timber.wav", 120, RESOURCE_TIMBER);
}

void UI_BuildingInfo_drawClayPit(BuildingInfoContext *c)
{
	drawRawMaterial(c, 92, "wavs/clay.wav", 121, RESOURCE_CLAY);
}

static void drawWorkshop(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId, int inputResourceId)
{
	c->helpId = helpId;
	window_building_play_sound(c, soundFile);

	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	image_draw(image_group(GROUP_RESOURCE_ICONS) + resourceId,
		c->xOffset + 10, c->yOffset + 10);
	lang_text_draw_centered(groupId, 0, c->xOffset, c->yOffset + 10,
		16 * c->widthBlocks, FONT_LARGE_BLACK);

    building *b = building_get(c->buildingId);
	int pctDone = calc_percentage(b->data.industry.progress, 400);
	int width = lang_text_draw(groupId, 2, c->xOffset + 32, c->yOffset + 40, FONT_NORMAL_BLACK);
	width += text_draw_percentage(pctDone,
		c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);
	lang_text_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);

	image_draw(image_group(GROUP_RESOURCE_ICONS) + inputResourceId,
		c->xOffset + 32, c->yOffset + 56);
	width = lang_text_draw(groupId, 12, c->xOffset + 60, c->yOffset + 60, FONT_NORMAL_BLACK);
	if (b->loadsStored < 1) {
		lang_text_draw_amount(8, 10, 0,
			c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_amount(8, 10, b->loadsStored,
			c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
	}

	if (!c->hasRoadAccess) {
		window_building_draw_description_at(c, 86, 69, 25);
	} else if (Data_CityInfo.resourceIndustryMothballed[resourceId]) {
		window_building_draw_description_at(c, 86, groupId, 4);
	} else if (b->numWorkers <= 0) {
		window_building_draw_description_at(c, 86, groupId, 5);
	} else if (b->loadsStored <= 0) {
		window_building_draw_description_at(c, 86, groupId, 11);
	} else if (c->workerPercentage >= 100) {
		window_building_draw_description_at(c, 86, groupId, 6);
	} else if (c->workerPercentage >= 75) {
		window_building_draw_description_at(c, 86, groupId, 7);
	} else if (c->workerPercentage >= 50) {
		window_building_draw_description_at(c, 86, groupId, 8);
	} else if (c->workerPercentage >= 25) {
		window_building_draw_description_at(c, 86, groupId, 9);
	} else {
		window_building_draw_description_at(c, 86, groupId, 10);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawWineWorkshop(BuildingInfoContext *c)
{
	drawWorkshop(c, 96, "wavs/wine_workshop.wav", 122, RESOURCE_WINE, RESOURCE_VINES);
}

void UI_BuildingInfo_drawOilWorkshop(BuildingInfoContext *c)
{
	drawWorkshop(c, 97, "wavs/oil_workshop.wav", 123, RESOURCE_OIL, RESOURCE_OLIVES);
}

void UI_BuildingInfo_drawWeaponsWorkshop(BuildingInfoContext *c)
{
	drawWorkshop(c, 98, "wavs/weapons_workshop.wav", 124, RESOURCE_WEAPONS, RESOURCE_IRON);
}

void UI_BuildingInfo_drawFurnitureWorkshop(BuildingInfoContext *c)
{
	drawWorkshop(c, 99, "wavs/furniture_workshop.wav", 125, RESOURCE_FURNITURE, RESOURCE_TIMBER);
}

void UI_BuildingInfo_drawPotteryWorkshop(BuildingInfoContext *c)
{
	drawWorkshop(c, 1, "wavs/pottery_workshop.wav", 126, RESOURCE_POTTERY, RESOURCE_CLAY);
}
