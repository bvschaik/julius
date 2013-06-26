#include "Advisors_private.h"

void UI_Advisor_Trade_drawBackground()
{
	// TODO fun_updateAvailableGoodsAndFoods()
	
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 27);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + Advisor_Trade + 1,
		baseOffsetX + 10, baseOffsetY + 10);

	Widget_GameText_draw(54, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);

	Widget_GameText_draw(54, 1, baseOffsetX + 400, baseOffsetY + 38, Font_NormalBlack, 0);
}

