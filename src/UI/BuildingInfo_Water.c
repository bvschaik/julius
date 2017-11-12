#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Terrain.h"
#include "../Widget.h"
#include "../Data/Building.h"

void UI_BuildingInfo_drawAqueduct(BuildingInfoContext *c)
{
	c->helpId = 60;
	PLAY_SOUND("wavs/aquaduct.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(141, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	if (c->aqueductHasWater) {
		Widget_GameText_drawMultiline(141, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		Widget_GameText_drawMultiline(141, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawReservoir(BuildingInfoContext *c)
{
	c->helpId = 59;
	PLAY_SOUND("wavs/resevoir.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(107, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
	if (Data_Buildings[c->buildingId].hasWaterAccess) {
		Widget_GameText_drawMultiline(107, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	} else {
		Widget_GameText_drawMultiline(107, 3,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}

void UI_BuildingInfo_drawFountain(BuildingInfoContext *c)
{
	c->helpId = 61;
	PLAY_SOUND("wavs/fountain.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(108, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	int textId;
	if (Data_Buildings[c->buildingId].hasWaterAccess) {
		if (Data_Buildings[c->buildingId].numWorkers > 0) {
			textId = 1;
		} else {
			textId = 2;
		}
	} else if (c->hasReservoirPipes) {
		textId = 2;
	} else {
		textId = 3;
	}
	Widget_GameText_drawMultiline(108, textId,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawWell(BuildingInfoContext *c)
{
	c->helpId = 62;
	PLAY_SOUND("wavs/well.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(109, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	int fountainAccess = Terrain_allHousesWithinWellRadiusHaveFountain(c->buildingId, 2);
	int textId = 0;
	if (fountainAccess == 0) { // well is OK
		textId = 1;
	} else if (fountainAccess == 1) { // all houses have fountain
		textId = 2;
	} else if (fountainAccess == 2) { // no houses around
		textId = 3;
	}
	if (textId) {
		Widget_GameText_drawMultiline(109, textId,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
			16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
	}
}
