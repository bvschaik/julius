#include "Advisors_private.h"

static void drawGeneralBackground();

enum {
	Advisor_None = 0,
	Advisor_Labor = 1,
	Advisor_Military = 2,
	Advisor_Imperial = 3,
	Advisor_Ratings = 4,
	Advisor_Trade = 5,
	Advisor_Population = 6,
	Advisor_Health = 7,
	Advisor_Education = 8,
	Advisor_Entertainment = 9,
	Advisor_Religion = 10,
	Advisor_Financial = 11,
	Advisor_Chief = 12
};

static const int dialogHeights[13] = {
	0, 0, 0, 0, 0, 0, 0, 18, 16, 0, 0, 0
};

static int currentAdvisor = Advisor_Education;

void UI_Advisors_drawBackground()
{
	drawGeneralBackground();
	switch (currentAdvisor) {
		case Advisor_Labor:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Military:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Imperial:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Ratings:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Trade:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Population:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Health:
			UI_Advisor_Health_drawBackground();
			break;
		case Advisor_Education:
			UI_Advisor_Education_drawBackground();
			break;
		case Advisor_Entertainment:
			// TODO UI_Advisor_Entertainment_drawBackground();
			break;
		case Advisor_Religion:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Financial:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Chief:
			// TODO UI_Advisor__drawBackground();
			break;
	}
}

void UI_Advisors_drawForeground()
{
	/* HELP BUTTON
        j_fun_drawImageButtonCollection(
          dialog_x,
          16 * (advisor_dialog_height - 2) + dialog_y,
          &imagebuttons_advisors,
          1);
	*/
	switch (currentAdvisor) {
		case Advisor_Labor:
			// TODO j_fun_drawArrowButtonCollection(dialog_x, dialog_y, &arrowbuttons_laborAdvisor, 2);j_fun_drawLaborAdvisorCategories();
			break;
		case Advisor_Military:
			// TODO j_fun_drawMilitaryAdvisorButtons();
			break;
		case Advisor_Imperial:
			// TODO j_fun_drawImperialAdvisorButtons();
			break;
		case Advisor_Ratings:
			// TODO j_fun_drawRatingsAdvisorButtons();
			break;
		case Advisor_Trade:
			// TODO j_fun_drawTradeAdvisorButtons();
			break;
		case Advisor_Population:
			// TODO j_fun_drawPopulationAdvisorButtons();
			break;
		case Advisor_Entertainment:
			// TODO j_fun_drawEntertainmentAdvisorHoldFestivalButton();
			break;
		case Advisor_Financial:
			// TODO j_fun_drawArrowButtonCollection(dialog_x, dialog_y, &arrowbuttons_financeAdvisor, 2);
			break;
	}
}

static void drawGeneralBackground()
{
	if (Data_Screen.width > 1024 || Data_Screen.height > 768) {
		Graphics_clearScreen();
	}
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorBackground),
		(Data_Screen.width - 1024) / 2, (Data_Screen.height - 768) / 2);

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	Graphics_drawImage(GraphicId(ID_Graphic_PanelWindows) + 13,
		baseOffsetX, baseOffsetY + 432
	);

	for (int i = 0; i < 13; i++) {
		int selected = 0;
		if (currentAdvisor && i == currentAdvisor - 1) {
			selected = 13;
		}
		Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + i + selected,
			baseOffsetX + 48 * i + 12, baseOffsetY + 441
		);
	}
/*
  for ( i = 0; i < 13; ++i )
  {
    if ( currentAdvisor )
    {
      if ( i == currentAdvisor - 1 )
        v0 = 13;
      else
        v0 = 0;
    }
    else
    {
      v0 = 0;
    }
    j_fun_drawGraphic(v0 + i + graphic_advisorIcons, screen_640x480_x + 48 * i + 12, screen_640x480_y + 441);
  }*/
}


