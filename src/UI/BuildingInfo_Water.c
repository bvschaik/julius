#include "BuildingInfo.h"

#include "building/building.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "map/water_supply.h"

void UI_BuildingInfo_drawAqueduct(BuildingInfoContext *c)
{
	c->helpId = 60;
	window_building_play_sound(c, "wavs/aquaduct.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(141, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 16 * c->heightBlocks - 128, 141, c->aqueductHasWater ? 1 : 2);
}

void UI_BuildingInfo_drawReservoir(BuildingInfoContext *c)
{
	c->helpId = 59;
	window_building_play_sound(c, "wavs/resevoir.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(107, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_centered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
    int text_id = building_get(c->buildingId)->hasWaterAccess ? 1 : 3;
    window_building_draw_description_at(c, 16 * c->heightBlocks - 173, 107, text_id);
}

void UI_BuildingInfo_drawFountain(BuildingInfoContext *c)
{
	c->helpId = 61;
	window_building_play_sound(c, "wavs/fountain.wav");
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
	window_building_draw_description_at(c, 16 * c->heightBlocks - 126, 108, textId);
}

void UI_BuildingInfo_drawWell(BuildingInfoContext *c)
{
	c->helpId = 62;
	window_building_play_sound(c, "wavs/well.wav");
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
        window_building_draw_description_at(c, 16 * c->heightBlocks - 126, 109, textId);
	}
}
