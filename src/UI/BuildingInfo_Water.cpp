#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"
#include "../Data/Building.h"

void UI_BuildingInfo_drawAqueduct(BuildingInfoContext *c)
{
	c->helpId = 60;
	PLAY_SOUND("wavs/aquaduct.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(141, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	if (c->aqueductHasWater) {
		Widget_GameText_drawMultiline(141, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(141, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 128,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_drawReservoir(BuildingInfoContext *c)
{
	c->helpId = 59;
	PLAY_SOUND("wavs/resevoir.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(107, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, Font_NormalBlack);
	if (Data_Buildings[c->buildingId].hasWaterAccess) {
		Widget_GameText_drawMultiline(107, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(107, 3,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 173,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_drawFountain(BuildingInfoContext *c)
{
	c->helpId = 61;
	PLAY_SOUND("wavs/fountain.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(108, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	if (Data_Buildings[c->buildingId].hasWaterAccess) {
		if (Data_Buildings[c->buildingId].numWorkers > 0) {
			Widget_GameText_drawMultiline(108, 1,
				c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
				16 * (c->heightBlocks - 4), Font_NormalBlack);
		} else {
			Widget_GameText_drawMultiline(108, 2,
				c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
				16 * (c->heightBlocks - 4), Font_NormalBlack);
		}
	} else if (c->hasReservoirPipes) {
		Widget_GameText_drawMultiline(108, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(108, 3,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 126,
			16 * (c->heightBlocks - 4), Font_NormalBlack);
	}
}
