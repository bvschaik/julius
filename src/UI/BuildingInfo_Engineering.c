#include "BuildingInfo.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

#include "../Graphics.h"

#include "../Data/CityInfo.h"

void UI_BuildingInfo_drawEngineersPost(BuildingInfoContext *c)
{
	c->helpId = 81;
	PLAY_SOUND("wavs/eng_post.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(104, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (!b->numWorkers) {
		DRAW_DESC(104, 9);
	} else {
		if (b->figureId) {
			DRAW_DESC(104, 2);
		} else {
			DRAW_DESC(104, 3);
		}
		if (c->workerPercentage >= 100) {
			DRAW_DESC_AT(72, 104, 4);
		} else if (c->workerPercentage >= 75) {
			DRAW_DESC_AT(72, 104, 5);
		} else if (c->workerPercentage >= 50) {
			DRAW_DESC_AT(72, 104, 6);
		} else if (c->workerPercentage >= 25) {
			DRAW_DESC_AT(72, 104, 7);
		} else {
			DRAW_DESC_AT(72, 104, 8);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawShipyard(BuildingInfoContext *c)
{
	c->helpId = 82;
	PLAY_SOUND("wavs/shipyard.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(100, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else {
		int pctDone = calc_percentage(b->data.industry.progress, 160);
		int width = lang_text_draw(100, 2,
			c->xOffset + 32, c->yOffset + 56, FONT_NORMAL_BLACK);
		width += text_draw_percentage(pctDone,
			c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
		lang_text_draw(100, 3,
			c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
		if (Data_CityInfo.shipyardBoatsRequested) {
			lang_text_draw_multiline(100, 5,
				c->xOffset + 32, c->yOffset + 80,
				16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
		} else {
			lang_text_draw_multiline(100, 4,
				c->xOffset + 32, c->yOffset + 80,
				16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawDock(BuildingInfoContext *c)
{
	c->helpId = 83;
	PLAY_SOUND("wavs/dock.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(101, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->data.other.boatFigureId) {
		if (c->workerPercentage <= 0) {
			DRAW_DESC(101, 2);
		} else if (c->workerPercentage < 50) {
			DRAW_DESC(101, 3);
		} else if (c->workerPercentage < 75) {
			DRAW_DESC(101, 4);
		} else {
			DRAW_DESC(101, 5);
		}
	} else {
		if (c->workerPercentage <= 0) {
			DRAW_DESC(101, 6);
		} else if (c->workerPercentage < 50) {
			DRAW_DESC(101, 7);
		} else if (c->workerPercentage < 75) {
			DRAW_DESC(101, 8);
		} else {
			DRAW_DESC(101, 9);
		}
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawWharf(BuildingInfoContext *c)
{
	c->helpId = 84;
	PLAY_SOUND("wavs/wharf.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(102, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_MEAT +
		resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
		c->xOffset + 10, c->yOffset + 10);

	building *b = building_get(c->buildingId);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (!b->data.other.boatFigureId) {
		DRAW_DESC(102, 2);
	} else {
		int boatId = b->data.other.boatFigureId;
		int textId;
		switch (figure_get(boatId)->actionState) {
			case FIGURE_ACTION_191_FISHING_BOAT_GOING_TO_FISH: textId = 3; break;
			case FIGURE_ACTION_192_FISHING_BOAT_FISHING: textId = 4; break;
			case FIGURE_ACTION_193_FISHING_BOAT_GOING_TO_WHARF: textId = 5; break;
			case FIGURE_ACTION_194_FISHING_BOAT_AT_WHARF: textId = 6; break;
			case FIGURE_ACTION_195_FISHING_BOAT_RETURNING_WITH_FISH: textId = 7; break;
			default: textId = 8; break;
		}
		DRAW_DESC(102, textId);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawBurningRuin(BuildingInfoContext *c)
{
	c->helpId = 0;
	PLAY_SOUND("wavs/ruin.wav");
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
	PLAY_SOUND("wavs/ruin.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(140, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	lang_text_draw(41, c->rubbleBuildingType,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
	lang_text_draw_multiline(140, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}
