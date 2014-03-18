#include "BuildingInfo.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"

void UI_BuildingInfo_drawGarden(BuildingInfoContext *c)
{
	c->helpId = 80;
	PLAY_SOUND("wavs/park.wav");
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(79, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawMultiline(79, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawPlaza(BuildingInfoContext *c)
{
	c->helpId = 80;
	PLAY_SOUND("wavs/plaza.wav");
	UI_BuildingInfo_drawWalkerListLocal(c);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(137, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	UI_BuildingInfo_drawWalkerList(c);
	Widget_GameText_drawMultiline(137, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawStatue(BuildingInfoContext *c)
{
	c->helpId = 79;
	PLAY_SOUND("wavs/statue.wav");
	UI_BuildingInfo_drawWalkerListLocal(c);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(80, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawMultiline(80, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawTriumphalArch(BuildingInfoContext *c)
{
	c->helpId = 79;
	PLAY_SOUND("wavs/statue.wav");
	UI_BuildingInfo_drawWalkerListLocal(c);
	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(80, 2, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, Font_LargeBlack);
	Widget_GameText_drawMultiline(80, 3,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

