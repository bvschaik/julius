#include "buildinginfo.h"

#include "sound.h"
#include "widget.h"

#include "data/figure.hpp"

#include "figure/type.h"

void UI_BuildingInfo_drawNoPeople(BuildingInfoContext *c)
{
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(70, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 22,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawTerrain(BuildingInfoContext *c)
{
	switch (c->terrainType) {
		case 6: c->helpId = 57; break;
		case 7: c->helpId = 60; break;
		case 9: c->helpId = 85; break;
		case 11: c->helpId = 58; break;
		default: c->helpId = 0; break;
	}

	if (c->terrainType == 7) {
		UI_BuildingInfo_drawAqueduct(c);
	} else if (c->terrainType == 8) {
		UI_BuildingInfo_drawRubble(c);
	} else if (c->terrainType == 9) {
		UI_BuildingInfo_drawWall(c);
	} else if (c->terrainType == 12) {
		UI_BuildingInfo_drawGarden(c);
	} else if (c->terrainType == 13 && c->figure.count <= 0) {
		UI_BuildingInfo_drawPlaza(c);
	} else {
		if (c->canPlaySound) {
			c->canPlaySound = 0;
			if (c->figure.count > 0) {
				UI_BuildingInfo_playFigurePhrase(c);
			} else {
				Sound_Speech_playFile("wavs/empty_land.wav");
			}
		}
		if (c->figure.count > 0 && c->figure.figureIds[c->figure.selectedIndex]) {
			if (Data_Figures[c->figure.figureIds[c->figure.selectedIndex]].type < FIGURE_SHIPWRECK) {
				c->helpId = 42;
			} else {
				c->helpId = 330;
			}
		}
		
		UI_BuildingInfo_drawFigureImagesLocal(c);
		Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset,
			c->widthBlocks, c->heightBlocks);
		if (!c->figure.count) {
			Widget_GameText_drawCentered(70, c->terrainType + 10,
				c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
		}
		if (c->terrainType != 6 && c->terrainType != 13) {
			Widget_GameText_drawMultiline(70, c->terrainType + 25,
				c->xOffset + 36, c->yOffset + 16 * c->heightBlocks - 113,
				16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
		}
		UI_BuildingInfo_drawFigureList(c);
	}
}
