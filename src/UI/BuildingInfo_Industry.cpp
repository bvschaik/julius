#include "BuildingInfo.h"
#include "../Calc.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"
#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/Graphics.h"
#include "../Data/Constants.h"

void drawFarm(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId)
{
	c->helpId = helpId;
	PLAY_SOUND(soundFile);

	Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
	Graphics_drawImage(GraphicId(ID_Graphic_ResourceIcons) + resourceId,
		c->xOffset + 10, c->yOffset + 10);
	Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10,
		16 * c->widthBlocks, Font_LargeBlack);

	int pctGrown = Calc_getPercentage(Data_Buildings[c->buildingId].data.industry.progress, 200);
	int width = Widget_GameText_draw(groupId, 2, c->xOffset + 32, c->yOffset + 44, Font_NormalBlack);
	width += Widget_Text_drawNumber(pctGrown, '@', "%",
		c->xOffset + 32 + width, c->yOffset + 44, Font_NormalBlack);
	Widget_GameText_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 44, Font_NormalBlack);

	if (!c->hasRoadAccess) {
		DRAW_DESC_AT(70, 69, 25);
	} else if (Data_CityInfo.resourceIndustryMothballed[resourceId]) {
		DRAW_DESC_AT(70, groupId, 4);
	} else if (Data_Buildings[c->buildingId].data.industry.curseWeeksLeft > 4) {
		DRAW_DESC_AT(70, groupId, 11);
	} else if (Data_Buildings[c->buildingId].numWorkers <= 0) {
		DRAW_DESC_AT(70, groupId, 5);
	} else if (c->workerPercentage >= 100) {
		DRAW_DESC_AT(70, groupId, 6);
	} else if (c->workerPercentage >= 75) {
		DRAW_DESC_AT(70, groupId, 7);
	} else if (c->workerPercentage >= 50) {
		DRAW_DESC_AT(70, groupId, 8);
	} else if (c->workerPercentage >= 25) {
		DRAW_DESC_AT(70, groupId, 9);
	} else {
		DRAW_DESC_AT(70, groupId, 10);
	}

	Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
	UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
	Widget_GameText_drawMultiline(groupId, 1,
		c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
		16 * (c->widthBlocks - 4), Font_NormalBlack);
}

void UI_BuildingInfo_drawWheatFarm(BuildingInfoContext *c)
{
	drawFarm(c, 89, "wavs/wheat_farm.wav", 112, Resource_Wheat);
}

void UI_BuildingInfo_drawVegetableFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/veg_farm.wav", 113, Resource_Vegetables);
}

void UI_BuildingInfo_drawFruitFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/figs_farm.wav", 114, Resource_Fruit);
}

void UI_BuildingInfo_drawOliveFarm(BuildingInfoContext *c)
{
	drawFarm(c, 91, "wavs/olives_farm.wav", 115, Resource_Olives);
}

void UI_BuildingInfo_drawVinesFarm(BuildingInfoContext *c)
{
	drawFarm(c, 91, "wavs/vines_farm.wav", 116, Resource_Vines);
}

void UI_BuildingInfo_drawPigFarm(BuildingInfoContext *c)
{
	drawFarm(c, 90, "wavs/meat_farm.wav", 117, Resource_Meat);
}

