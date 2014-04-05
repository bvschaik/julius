#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"
#include "../Data/Building.h"
#include "../Data/Formation.h"
#include "../Data/Constants.h"

void UI_BuildingInfo_drawWall(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/wall.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(139, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawMultiline(139, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawPrefect(BuildingInfoContext *c)
{
	c->helpId = 86;
	PLAY_SOUND("wavs/prefecture.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(88, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 24,
		16 * c->widthBlocks, Font_NormalBlack);

	Data_Building *b = &Data_Buildings[c->buildingId];
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(88, 9);
	} else {
		if (b->walkerId) {
			DRAW_DESC(88, 2);
		} else {
			DRAW_DESC(88, 3);
		}
		if (c->workerPercentage >= 100) {
			DRAW_DESC_AT(72, 88, 4);
		} else if (c->workerPercentage >= 75) {
			DRAW_DESC_AT(72, 88, 5);
		} else if (c->workerPercentage >= 50) {
			DRAW_DESC_AT(72, 88, 6);
		} else if (c->workerPercentage >= 25) {
			DRAW_DESC_AT(72, 88, 7);
		} else {
			DRAW_DESC_AT(72, 88, 8);
		}
	}

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawFort(BuildingInfoContext *c)
{
	c->helpId = 87;
	PLAY_SOUND("wavs/fort.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(89, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	if (Data_Formations[c->formationId].cursedByMars) {
		Widget_GameText_drawMultiline(89, 1,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
			16 * (c->widthBlocks - 4), Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(89, 2,
			c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
			16 * (c->widthBlocks - 4), Font_NormalBlack);
	}
}

void UI_BuildingInfo_drawGatehouse(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/gatehouse.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(90, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	Widget_GameText_drawMultiline(90, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawTower(BuildingInfoContext *c)
{
	c->helpId = 85;
	PLAY_SOUND("wavs/tower.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(91, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (Data_Buildings[c->buildingId].numWorkers <= 0) {
		DRAW_DESC(91, 2);
	} else if (Data_Buildings[c->buildingId].walkerId) {
		DRAW_DESC(91, 3);
	} else {
		DRAW_DESC(91, 4);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawMilitaryAcademy(BuildingInfoContext *c)
{
	c->helpId = 88;
	PLAY_SOUND("wavs/mil_acad.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(135, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);

	Data_Building *b = &Data_Buildings[c->buildingId];
	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC(135, 2);
	} else if (c->workerPercentage >= 100) {
		DRAW_DESC(135, 1);
	} else {
		DRAW_DESC(135, 3);
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawBarracks(BuildingInfoContext *c)
{
	c->helpId = 37;
	PLAY_SOUND("wavs/barracks.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(136, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + Resource_Weapons,
		c->xOffset + 64, c->yOffset + 38);

	Data_Building *b = &Data_Buildings[c->buildingId];
	if (b->loadsStored < 1) {
		Widget_GameText_drawNumberWithDescription(8, 10, 0,
			c->xOffset + 92, c->yOffset + 44, Font_NormalBlack);
	} else {
		Widget_GameText_drawNumberWithDescription(8, 10, b->loadsStored,
			c->xOffset + 92, c->yOffset + 44, Font_NormalBlack);
	}

	if (!c->hasRoadAccess) {
		DRAW_DESC_AT(70, 69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC_AT(70, 136, 3);
	} else if (!c->barracksSoldiersRequested) {
		DRAW_DESC_AT(70, 136, 4);
	} else {
		int offset = 0;
		if (b->loadsStored > 0) {
			offset = 4;
		}
		if (c->workerPercentage >= 100) {
			DRAW_DESC_AT(70, 136, 5 + offset);
		} else if (c->workerPercentage >= 66) {
			DRAW_DESC_AT(70, 136, 6 + offset);
		} else if (c->workerPercentage >= 33) {
			DRAW_DESC_AT(70, 136, 7 + offset);
		} else {
			DRAW_DESC_AT(70, 136, 8 + offset);
		}
	}
	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

