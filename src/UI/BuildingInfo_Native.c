#include "BuildingInfo.h"

#include "graphics/lang_text.h"
#include "graphics/panel.h"

static void drawNative(BuildingInfoContext *c, int groupId)
{
	c->helpId = 0;
	window_building_play_sound(c, "wavs/empty_land.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(groupId, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	window_building_draw_description_at(c, 106, groupId, 1);
}

void UI_BuildingInfo_drawNativeHut(BuildingInfoContext *c)
{
	drawNative(c, 131);
}

void UI_BuildingInfo_drawNativeMeeting(BuildingInfoContext *c)
{
	drawNative(c, 132);
}

void UI_BuildingInfo_drawNativeCrops(BuildingInfoContext *c)
{
	drawNative(c, 133);
}

void UI_BuildingInfo_drawMissionPost(BuildingInfoContext *c)
{
	c->helpId = 8;
	window_building_play_sound(c, "wavs/mission.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(134, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    window_building_draw_description_at(c, 106, 134, 1);
}
