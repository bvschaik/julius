#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"

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

