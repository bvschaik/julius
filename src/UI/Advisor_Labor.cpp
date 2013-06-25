#include "Advisors_private.h"
#include "../Data/Mouse.h"

static void arrowButtonWages(int param1, int param2);


static ArrowButton wageButtons[2] = {
	{158, 354, 17, 24, arrowButtonWages, 1, 0},
	{182, 354, 15, 24, arrowButtonWages, 0, 0}
};

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

void UI_Advisor_Labor_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Button_drawArrowButtons(
		baseOffsetX, baseOffsetY, wageButtons, 2);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 70, 36, 15);

	for (int i = 0; i < 9; i++) {
		int focus = i == Data_Mouse.focusButtonId;
		Widget_Panel_drawButtonBorder(
			baseOffsetX + 40, baseOffsetY + 77 + 25 * i,
			560, 22, focus
		);
		if (0) {
			Graphics_drawImage(GraphicId(ID_Graphic_LaborPriorityLock),
				baseOffsetX + 70, baseOffsetY + 80 + 25 * i);
			//Widget_Text_drawNumber();
		}
		int width = Widget_GameText_draw(50, i + 1,
			baseOffsetX + 170, baseOffsetY + 82 + 25 * i, Font_NormalWhite, 0);

	}
/*
    for ( i = 0; i < 9; ++i )
    {
      focus = i == mouseover_button_id - 1;
      j_fun_drawBorderedButton(0, dialog_x + 40, dialog_y + 25 * i + 77, 560, 22, focus);
      if ( *(int *)((char *)&cityinfo_labor_category_priority[4517 * ciid] + 20 * i) )
      {
        j_fun_drawGraphic(graphic_laborPriorityLock, dialog_x + 70, dialog_y + 25 * i + 80);
        j_fun_drawNumber(
          *(int *)((char *)&cityinfo_labor_category_priority[4517 * ciid] + 20 * i),
          64,
          L" ",
          dialog_x + 90,
          dialog_y + 25 * i + 82,
          graphic_font + F_NormalWhite,
          0);
      }
      text_xoffset = 0;
      j_fun_drawGameText(50, i + 1, dialog_x + 170, dialog_y + 25 * i + 82, graphic_font + F_NormalWhite, 0);
      j_fun_drawNumber(
        *(int *)((char *)&cityinfo_labor_category_numWorkersNeeded[4517 * ciid] + 20 * i),
        64,
        L" ",
        dialog_x + 410,
        dialog_y + 25 * i + 82,
        graphic_font + F_NormalWhite,
        0);
      font = graphic_font + F_NormalWhite;
      if ( *(int *)((char *)&cityinfo_labor_category_workersAllocated[4517 * ciid] + 20 * i) != *(int *)((char *)&cityinfo_labor_category_numWorkersNeeded[4517 * ciid] + 20 * i) )
        font = graphic_font + F_NormalRed;
      j_fun_drawNumber(
        *(int *)((char *)&cityinfo_labor_category_workersAllocated[4517 * ciid] + 20 * i),
        64,
        L" ",
        dialog_x + 510,
        dialog_y + 25 * i + 82,
        font,
        0);
    }
*/
}


static void arrowButtonWages(int param1, int param2)
{
	// TODO
}
