#include "BuildingInfo.h"

#include "graphics/lang_text.h"
#include "graphics/panel.h"

void UI_BuildingInfo_drawGarden(BuildingInfoContext *c)
{
	c->helpId = 80;
	PLAY_SOUND("wavs/park.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(79, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_multiline(79, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawPlaza(BuildingInfoContext *c)
{
	c->helpId = 80;
	PLAY_SOUND("wavs/plaza.wav");
	UI_BuildingInfo_drawFigureImagesLocal(c);
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(137, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	UI_BuildingInfo_drawFigureList(c);
	lang_text_draw_multiline(137, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawStatue(BuildingInfoContext *c)
{
	c->helpId = 79;
	PLAY_SOUND("wavs/statue.wav");
	UI_BuildingInfo_drawFigureImagesLocal(c);
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(80, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_multiline(80, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawTriumphalArch(BuildingInfoContext *c)
{
	c->helpId = 79;
	PLAY_SOUND("wavs/statue.wav");
	UI_BuildingInfo_drawFigureImagesLocal(c);
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	lang_text_draw_centered(80, 2, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	lang_text_draw_multiline(80, 3,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

