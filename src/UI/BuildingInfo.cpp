#include "BuildingInfo.h"

#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Model.h"

void UI_BuildingInfo_drawEmploymentInfo(BuildingInfoContext *c, int yOffset)
{
	Data_Building *b = &Data_Buildings[c->buildingId];
	int textId;
	if (b->numWorkers >= Data_Model_Buildings[b->type].laborers) {
		textId = 0;
	} else if (Data_CityInfo.population <= 0) {
		textId = 16; // no people in city
	} else if (b->housesCovered <= 0) {
		textId = 17; // no employees nearby
	} else if (b->housesCovered < 40) {
		textId = 20; // poor access to employees
	} else if (Data_CityInfo.laborCategory[b->laborCategory].workersAllocated > 0) {
		textId = 18; // no people allocated
	} else {
		textId = 19; // too few people allocated
	}
	if (!textId && b->housesCovered < 40) {
		textId = 20; // poor access to employees
	}
	Graphics_drawImage(GraphicId(ID_Graphic_ContextIcons) + 14,
		c->xOffset + 40, yOffset + 6);
	if (textId) {
		int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 10, Font_SmallBlack);
		width += Widget_Text_drawNumber(Data_Model_Buildings[b->type].laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 10, Font_SmallBlack);
		Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 10, Font_SmallBlack);
		Widget_GameText_draw(69, textId, c->xOffset + 70, yOffset + 26, Font_SmallBlack);
	} else {
		int width = Widget_GameText_drawNumberWithDescription(8, 12, b->numWorkers,
			c->xOffset + 60, yOffset + 16, Font_SmallBlack);
		width += Widget_Text_drawNumber(Data_Model_Buildings[b->type].laborers, '(', "",
			c->xOffset + 70 + width, yOffset + 16, Font_SmallBlack);
		Widget_GameText_draw(69, 0, c->xOffset + 70 + width, yOffset + 16, Font_SmallBlack);
	}
}
