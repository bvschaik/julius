#include "BuildingInfo.h"

#include "../Graphics.h"

#include "building/building.h"
#include "game/resource.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

void UI_BuildingInfo_drawForum(BuildingInfoContext *c)
{
	c->helpId = 76;
	PLAY_SOUND("wavs/forum.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(106, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_DENARII,
		c->xOffset + 16, c->yOffset + 36);

	building *b = building_get(c->buildingId);
	int width = Widget_GameText_draw(106, 2,
		c->xOffset + 44, c->yOffset + 43, FONT_NORMAL_BLACK);
	Widget_GameText_drawNumberWithDescription(8, 0, b->taxIncomeOrStorage,
		c->xOffset + 44 + width, c->yOffset + 43, FONT_NORMAL_BLACK);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC_AT(72, 106, 10);
	} else if (c->workerPercentage >= 100) {
		DRAW_DESC_AT(72, 106, 5);
	} else if (c->workerPercentage >= 75) {
		DRAW_DESC_AT(72, 106, 6);
	} else if (c->workerPercentage >= 50) {
		DRAW_DESC_AT(72, 106, 7);
	} else if (c->workerPercentage >= 25) {
		DRAW_DESC_AT(72, 106, 8);
	} else {
		DRAW_DESC_AT(72, 106, 9);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawSenate(BuildingInfoContext *c)
{
	c->advisor = 1;
	c->helpId = 77;
	PLAY_SOUND("wavs/senate.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(105, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Graphics_drawImage(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_DENARII,
		c->xOffset + 16, c->yOffset + 36);

	building *b = building_get(c->buildingId);
	int width = Widget_GameText_draw(105, 2,
		c->xOffset + 44, c->yOffset + 43, FONT_NORMAL_BLACK);
	Widget_GameText_drawNumberWithDescription(8, 0, b->taxIncomeOrStorage,
		c->xOffset + 44 + width, c->yOffset + 43, FONT_NORMAL_BLACK);

	if (!c->hasRoadAccess) {
		DRAW_DESC(69, 25);
	} else if (b->numWorkers <= 0) {
		DRAW_DESC_AT(72, 106, 10);
	} else if (c->workerPercentage >= 100) {
		DRAW_DESC_AT(72, 106, 5);
	} else if (c->workerPercentage >= 75) {
		DRAW_DESC_AT(72, 106, 6);
	} else if (c->workerPercentage >= 50) {
		DRAW_DESC_AT(72, 106, 7);
	} else if (c->workerPercentage >= 25) {
		DRAW_DESC_AT(72, 106, 8);
	} else {
		DRAW_DESC_AT(72, 106, 9);
	}

	inner_panel_draw(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);

	Widget_GameText_draw(105, 3, c->xOffset + 60, c->yOffset + 220, FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawGovernorsHome(BuildingInfoContext *c)
{
	c->helpId = 78;
	PLAY_SOUND("wavs/gov_palace.wav");
	outer_panel_draw(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Widget_GameText_drawCentered(103, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
	Widget_GameText_drawMultiline(103, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 143,
		16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}
