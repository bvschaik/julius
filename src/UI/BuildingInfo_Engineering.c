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
