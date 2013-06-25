#include "Advisors_private.h"

void UI_Advisor_Labor_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 26);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons), baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(50, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);
	
	// table headers
	Widget_GameText_draw(50, 21, baseOffsetX + 60, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 22, baseOffsetX + 170, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 23, baseOffsetX + 400, baseOffsetY + 56, Font_SmallPlain, 0);
	Widget_GameText_draw(50, 24, baseOffsetX + 500, baseOffsetY + 56, Font_SmallPlain, 0);
	
	int width = Widget_Text_drawNumber(Data_CityInfo.workersEmployed, '@', " ",
		baseOffsetX + 32, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_GameText_draw(50, 12,
		baseOffsetX + 32 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.workersUnemployed, '@', " ",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_GameText_draw(50, 13,
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.unemploymentPercentage, '@', "%)",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack, 0
	);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 350, 32, 2);
	Widget_GameText_draw(50, 14, baseOffsetX + 70, baseOffsetY + 359, Font_NormalWhite, 0);
	
	width = Widget_Text_drawNumber(Data_CityInfo.wages, '@', " ",
		baseOffsetX + 230, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_GameText_draw(50, 15,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_GameText_draw(50, 18,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.wagesRome, '@', " )",
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite, 0
	);
	
	width = Widget_GameText_draw(50, 19,
		baseOffsetX + 64, baseOffsetY + 390, Font_NormalBlack, 0
	);
	width += Widget_Text_drawNumber(Data_CityInfo.estimatedYearlyWages, '@', " Dn",
		baseOffsetX + 64 + width, baseOffsetY + 390, Font_NormalBlack, 0
	);
}
