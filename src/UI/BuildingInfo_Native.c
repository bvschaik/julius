#include "BuildingInfo.h"
#include "../Widget.h"

#include "graphics/panel.h"

static void drawNative(BuildingInfoContext *c, int groupId)
{
	c->helpId = 0;
	PLAY_SOUND("wavs/empty_land.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Widget_GameText_drawMultiline(groupId, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
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
	PLAY_SOUND("wavs/mission.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(134, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Widget_GameText_drawMultiline(134, 1,
		c->xOffset + 32, c->yOffset + 106,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}
