#include "BuildingInfo.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#include "../Data/CityInfo.h"

void UI_BuildingInfo_drawEngineersPost(BuildingInfoContext *c)
{
	c->helpId = 81;
	window_building_play_sound(c, "wavs/eng_post.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(104, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		window_building_draw_description(c, 69, 25);
	} else if (!b->numWorkers) {
		window_building_draw_description(c, 104, 9);
	} else {
		if (b->figureId) {
			window_building_draw_description(c, 104, 2);
		} else {
			window_building_draw_description(c, 104, 3);
		}
		if (c->workerPercentage >= 100) {
			window_building_draw_description_at(c, 72, 104, 4);
		} else if (c->workerPercentage >= 75) {
			window_building_draw_description_at(c, 72, 104, 5);
		} else if (c->workerPercentage >= 50) {
			window_building_draw_description_at(c, 72, 104, 6);
		} else if (c->workerPercentage >= 25) {
			window_building_draw_description_at(c, 72, 104, 7);
		} else {
			window_building_draw_description_at(c, 72, 104, 8);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, 142);
}

void UI_BuildingInfo_drawDock(BuildingInfoContext *c)
{
	c->helpId = 83;
	window_building_play_sound(c, "wavs/dock.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(101, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		window_building_draw_description(c, 69, 25);
	} else if (b->data.other.boatFigureId) {
		if (c->workerPercentage <= 0) {
			window_building_draw_description(c, 101, 2);
		} else if (c->workerPercentage < 50) {
			window_building_draw_description(c, 101, 3);
		} else if (c->workerPercentage < 75) {
			window_building_draw_description(c, 101, 4);
		} else {
			window_building_draw_description(c, 101, 5);
		}
	} else {
		if (c->workerPercentage <= 0) {
			window_building_draw_description(c, 101, 6);
		} else if (c->workerPercentage < 50) {
			window_building_draw_description(c, 101, 7);
		} else if (c->workerPercentage < 75) {
			window_building_draw_description(c, 101, 8);
		} else {
			window_building_draw_description(c, 101, 9);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	window_building_draw_employment(c, 142);
}

void UI_BuildingInfo_drawBurningRuin(BuildingInfoContext *c)
{
	c->helpId = 0;
	window_building_play_sound(c, "wavs/ruin.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(111, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	lang_text_draw(41, c->rubbleBuildingType,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
	lang_text_draw_multiline(111, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawRubble(BuildingInfoContext *c)
{
	c->helpId = 0;
	window_building_play_sound(c, "wavs/ruin.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(140, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	lang_text_draw(41, c->rubbleBuildingType,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
	lang_text_draw_multiline(140, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}
