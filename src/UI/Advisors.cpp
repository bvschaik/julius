#include "Advisors_private.h"
#include "Window.h"

static void drawGeneralBackground();
static void buttonChangeAdvisor(int param1, int param2);

static CustomButton advisorButtons[13] = {
	{12, 1, 52, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 1, 0},
	{60, 1, 100, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 2, 0},
	{108, 1, 148, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 3, 0},
	{156, 1, 196, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 4, 0},
	{204, 1, 244, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 5, 0},
	{252, 1, 292, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 6, 0},
	{300, 1, 340, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 7, 0},
	{348, 1, 388, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 8, 0},
	{396, 1, 436, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 9, 0},
	{444, 1, 484, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 10, 0},
	{492, 1, 532, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 11, 0},
	{540, 1, 580, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 12, 0},
	{588, 1, 624, 41, buttonChangeAdvisor, Widget_Button_doNothing, 1, 0, 0},
};

static const int dialogHeights[13] = {
	0, 0, 0, 0, 0, 0, 0, 18, 16, 0, 0, 0, 0
};

static int currentAdvisor = Advisor_Education;

static int focusButtonId;

void UI_Advisors_drawBackground()
{
	drawGeneralBackground();
	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_drawBackground();
			break;
		case Advisor_Military:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Imperial:
			// TODO UI_Advisor__drawBackground();
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_drawBackground();
			break;
		case Advisor_Trade:
			UI_Advisor_Trade_drawBackground();
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
			UI_Advisor_Entertainment_drawBackground();
			break;
		case Advisor_Religion:
			UI_Advisor_Religion_drawBackground();
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
			UI_Advisor_Labor_drawForeground();
			break;
		case Advisor_Military:
			// TODO j_fun_drawMilitaryAdvisorButtons();
			break;
		case Advisor_Imperial:
			// TODO j_fun_drawImperialAdvisorButtons();
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_drawForeground();
			break;
		case Advisor_Trade:
			// TODO j_fun_drawTradeAdvisorButtons();
			break;
		case Advisor_Population:
			// TODO j_fun_drawPopulationAdvisorButtons();
			break;
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_drawForeground();
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

void UI_Advisors_handleMouse()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	if (Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY + 440, advisorButtons, 13, &focusButtonId)) {
		return;
	}

	// TODO help button
	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_handleMouse();
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_handleMouse();
			break;
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_handleMouse();
			break;

		// TODO other advisors
	}
}


static void buttonChangeAdvisor(int param1, int param2)
{
	if (param1) {
		currentAdvisor = param1;
		UI_Window_requestRefresh();
	} else {
		UI_Window_goTo(Window_City);
	}
}

