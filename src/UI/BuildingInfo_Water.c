#include "BuildingInfo.h"
#include "../Graphics.h"

#include "building/building.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "map/water_supply.h"

void UI_BuildingInfo_drawAqueduct(BuildingInfoContext *c)
{
	c->helpId = 60;
	PLAY_SOUND("wavs/aquaduct.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(141, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	if (c->aqueductHasWater) {
		lang_text_draw_multiline(141, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_multiline(141, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawReservoir(BuildingInfoContext *c)
{
	c->helpId = 59;
	PLAY_SOUND("wavs/resevoir.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(107, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_centered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
	if (building_get(c->buildingId)->hasWaterAccess) {
		lang_text_draw_multiline(107, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_multiline(107, 3,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawFountain(BuildingInfoContext *c)
{
	c->helpId = 61;
	PLAY_SOUND("wavs/fountain.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(108, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	int textId;
    building *b = building_get(c->buildingId);
	if (b->hasWaterAccess) {
		if (b->numWorkers > 0) {
			textId = 1;
		} else {
			textId = 2;
		}
	} else if (c->hasReservoirPipes) {
		textId = 2;
	} else {
		textId = 3;
	}
	lang_text_draw_multiline(108, textId,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawWell(BuildingInfoContext *c)
{
	c->helpId = 62;
	PLAY_SOUND("wavs/well.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(109, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	int wellNecessity = map_water_supply_is_well_unnecessary(c->buildingId, 2);
	int textId = 0;
	if (wellNecessity == WELL_NECESSARY) { // well is OK
		textId = 1;
	} else if (wellNecessity == WELL_UNNECESSARY_FOUNTAIN) { // all houses have fountain
		textId = 2;
	} else if (wellNecessity == WELL_UNNECESSARY_NO_HOUSES) { // no houses around
		textId = 3;
	}
	if (textId) {
		lang_text_draw_multiline(109, textId,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}
