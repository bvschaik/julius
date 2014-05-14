#include "BuildingInfo.h"

#include "../Widget.h"

void UI_BuildingInfo_drawNoPeople(BuildingInfoContext *c)
{
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(70, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawCentered(13, 1,
		c->xOffset, c->yOffset + 16 * c->heightBlocks - 22,
		16 * c->widthBlocks, Font_NormalBlack);
}

void UI_BuildingInfo_drawTerrain(BuildingInfoContext *c)
{
	// TODO
}
