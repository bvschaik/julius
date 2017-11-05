#include "buildinginfo.h"

#include "core/calc.h"
#include "graphics.h"
#include "resource.h"
#include "sound.h"
#include "widget.h"

#include "data/building.hpp"
#include "data/cityinfo.hpp"
#include "data/constants.hpp"
#include "data/figure.hpp"

void UI_BuildingInfo_drawEngineersPost(BuildingInfoContext *c)
{
	c->helpId = 81;
	PLAY_SOUND("wavs/eng_post.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(104, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	struct Data_Building *b = &Data_Buildings[c->buildingId];

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

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawShipyard(BuildingInfoContext *c)
{
	c->helpId = 82;
	PLAY_SOUND("wavs/shipyard.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(100, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	struct Data_Building *b = &Data_Buildings[c->buildingId];

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else {
		int pctDone = calc_percentage(b->data.industry.progress, 160);
		int width = Widget_GameText_draw(100, 2,
			c->xOffset + 32, c->yOffset + 56, FONT_NORMAL_BLACK);
		width += Widget_Text_drawPercentage(pctDone,
			c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
		Widget_GameText_draw(100, 3,
			c->xOffset + 32 + width, c->yOffset + 56, FONT_NORMAL_BLACK);
		if (Data_CityInfo.shipyardBoatsRequested) {
			Widget_GameText_drawMultiline(100, 5,
				c->xOffset + 32, c->yOffset + 80,
				16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
		} else {
			Widget_GameText_drawMultiline(100, 4,
				c->xOffset + 32, c->yOffset + 80,
				16 * (c->widthBlocks - 6), FONT_NORMAL_BLACK);
		}
	}

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawDock(BuildingInfoContext *c)
{
	c->helpId = 83;
	PLAY_SOUND("wavs/dock.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(101, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	struct Data_Building *b = &Data_Buildings[c->buildingId];

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

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawWharf(BuildingInfoContext *c)
{
	c->helpId = 84;
	PLAY_SOUND("wavs/wharf.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(102, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + Resource_Meat +
		Resource_getGraphicIdOffset(Resource_Meat, 3),
		c->xOffset + 10, c->yOffset + 10);

	struct Data_Building *b = &Data_Buildings[c->buildingId];

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (!b->data.other.boatFigureId) {
		DRAW_DESC(102, 2);
	} else {
		int boatId = b->data.other.boatFigureId;
		int textId;
		switch (Data_Figures[boatId].actionState) {
			case FigureActionState_191_FishingBoatGoingToFish: textId = 3; break;
			case FigureActionState_192_FishingBoatFishing: textId = 4; break;
			case FigureActionState_193_FishingBoatSailingToWharf: textId = 5; break;
			case FigureActionState_194_FishingBoatAtWharf: textId = 6; break;
			case FigureActionState_195_FishingBoatReturningWithFish: textId = 7; break;
			default: textId = 8; break;
		}
		DRAW_DESC(102, textId);
	}

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawBurningRuin(BuildingInfoContext *c)
{
	c->helpId = 0;
	PLAY_SOUND("wavs/ruin.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(111, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	Widget_GameText_draw(41, c->rubbleBuildingType,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
	Widget_GameText_drawMultiline(111, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawRubble(BuildingInfoContext *c)
{
	c->helpId = 0;
	PLAY_SOUND("wavs/ruin.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(140, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);

	Widget_GameText_draw(41, c->rubbleBuildingType,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173, FONT_NORMAL_BLACK);
	Widget_GameText_drawMultiline(140, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}
