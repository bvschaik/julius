#include "BuildingInfo.h"

#include "figure/figure.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "sound/speech.h"
#include "window/building/government.h"
#include "window/building/utility.h"

void UI_BuildingInfo_drawNoPeople(BuildingInfoContext *c)
{
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(70, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_centered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 22,
		16 * c->widthBlocks, FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawTerrain(BuildingInfoContext *c)
{
	switch (c->terrainType) {
		case TERRAIN_INFO_ROAD: c->helpId = 57; break;
		case TERRAIN_INFO_AQUEDUCT: c->helpId = 60; break;
		case TERRAIN_INFO_WALL: c->helpId = 85; break;
		case TERRAIN_INFO_BRIDGE: c->helpId = 58; break;
		default: c->helpId = 0; break;
	}

	if (c->terrainType == TERRAIN_INFO_AQUEDUCT) {
		window_building_draw_aqueduct(c);
	} else if (c->terrainType == TERRAIN_INFO_RUBBLE) {
		window_building_draw_rubble(c);
	} else if (c->terrainType == TERRAIN_INFO_WALL) {
		UI_BuildingInfo_drawWall(c);
	} else if (c->terrainType == TERRAIN_INFO_GARDEN) {
		window_building_draw_garden(c);
	} else if (c->terrainType == TERRAIN_INFO_PLAZA && c->figure.count <= 0) {
		window_building_draw_plaza(c);
	} else {
		if (c->canPlaySound) {
			c->canPlaySound = 0;
			if (c->figure.count > 0) {
				UI_BuildingInfo_playFigurePhrase(c);
			} else {
				sound_speech_play_file("wavs/empty_land.wav");
			}
		}
		if (c->figure.count > 0 && c->figure.figureIds[c->figure.selectedIndex]) {
            figure *f = figure_get(c->figure.figureIds[c->figure.selectedIndex]);
			if (f->type < FIGURE_SHIPWRECK) {
				c->helpId = 42;
			} else {
				c->helpId = 330;
			}
		}
		
		UI_BuildingInfo_drawFigureImagesLocal(c);
		outer_panel_draw(c->xOffset, c->yOffset,
			c->widthBlocks, c->heightBlocks);
		if (!c->figure.count) {
			lang_text_draw_centered(70, c->terrainType + 10,
				c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
		}
		if (c->terrainType != TERRAIN_INFO_ROAD && c->terrainType != TERRAIN_INFO_PLAZA) {
			lang_text_draw_multiline(70, c->terrainType + 25,
				c->xOffset + 36, c->yOffset + 16 * c->heightBlocks - 113,
				16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
		}
		UI_BuildingInfo_drawFigureList(c);
	}
}
