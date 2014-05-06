#include "Advisors_private.h"

#include "AllWindows.h"
#include "Window.h"
#include "MessageDialog.h"

#include "../CityInfo.h"
#include "../Formation.h"

static void buttonChangeAdvisor(int param1, int param2);
static void buttonHelp(int param1, int param2);

static ImageButton helpButton = {
	11, -7, 27, 27, 4, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0
};

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

static const int advisorToMessageTextId[] = {
	0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

static int currentAdvisor = Advisor_Education;

static int focusButtonId;
static int advisorHeight;

void UI_Advisors_setAdvisor(int advisor)
{
	currentAdvisor = advisor;
	UI_Advisors_init();
}

void UI_Advisors_init()
{
	CityInfo_Labor_allocateWorkersToCategories();
	CityInfo_Labor_allocateWorkersToBuildings();

	CityInfo_Finance_calculateEstimatedTaxes();
	CityInfo_Finance_calculateEstimatedWages();
	CityInfo_Finance_updateInterest();
	CityInfo_Finance_updateSalary();
	CityInfo_Finance_calculateTotals();

	CityInfo_Culture_calculateDemandsForAdvisors();
	CityInfo_Culture_updateCoveragePercentages();

	CityInfo_Resource_calculateFoodAndSupplyRomeWheat();
	Formation_calculateWalkers();

	CityInfo_Ratings_updateCultureExplanation();
	CityInfo_Ratings_updateProsperityExplanation();
	CityInfo_Ratings_updatePeaceExplanation();
	CityInfo_Ratings_updateFavorExplanation();

	switch (currentAdvisor) {
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_init();
	}
}

void UI_Advisors_drawBackground()
{
	UI_Advisor_drawGeneralBackground();
	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_drawBackground(&advisorHeight);
			break;
		case Advisor_Military:
			UI_Advisor_Military_drawBackground(&advisorHeight);
			break;
		case Advisor_Imperial:
			UI_Advisor_Imperial_drawBackground(&advisorHeight);
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_drawBackground(&advisorHeight);
			break;
		case Advisor_Trade:
			UI_Advisor_Trade_drawBackground(&advisorHeight);
			break;
		case Advisor_Population:
			UI_Advisor_Population_drawBackground(&advisorHeight);
			break;
		case Advisor_Health:
			UI_Advisor_Health_drawBackground(&advisorHeight);
			break;
		case Advisor_Education:
			UI_Advisor_Education_drawBackground(&advisorHeight);
			break;
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_drawBackground(&advisorHeight);
			break;
		case Advisor_Religion:
			UI_Advisor_Religion_drawBackground(&advisorHeight);
			break;
		case Advisor_Financial:
			UI_Advisor_Financial_drawBackground(&advisorHeight);
			break;
		case Advisor_Chief:
			UI_Advisor_Chief_drawBackground(&advisorHeight);
			break;
	}
}

void UI_Advisors_drawForeground()
{
	Widget_Button_drawImageButtons(Data_Screen.offset640x480.x,
		Data_Screen.offset640x480.y + 16 * (advisorHeight - 2),
		&helpButton, 1);

	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_drawForeground();
			break;
		case Advisor_Military:
			UI_Advisor_Military_drawForeground();
			break;
		case Advisor_Imperial:
			UI_Advisor_Imperial_drawForeground();
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_drawForeground();
			break;
		case Advisor_Trade:
			UI_Advisor_Trade_drawForeground();
			break;
		case Advisor_Population:
			UI_Advisor_Population_drawForeground();
			break;
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_drawForeground();
			break;
		case Advisor_Financial:
			UI_Advisor_Financial_drawForeground();
			break;
	}
}

void UI_Advisor_drawGeneralBackground()
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
}

void UI_Advisors_handleMouse()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	if (Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY + 440, advisorButtons, 13, &focusButtonId)) {
		return;
	}

	if (Widget_Button_handleImageButtons(baseOffsetX, baseOffsetY + 16 * (advisorHeight - 2), &helpButton, 1)) {
		return;
	}

	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_handleMouse();
			break;
		case Advisor_Military:
			UI_Advisor_Military_handleMouse();
		case Advisor_Imperial:
			UI_Advisor_Imperial_handleMouse();
			break;
		case Advisor_Ratings:
			UI_Advisor_Ratings_handleMouse();
			break;
		case Advisor_Trade:
			UI_Advisor_Trade_handleMouse();
			break;
		case Advisor_Population:
			UI_Advisor_Population_handleMouse();
			break;
		case Advisor_Entertainment:
			UI_Advisor_Entertainment_handleMouse();
			break;
		case Advisor_Financial:
			UI_Advisor_Financial_handleMouse();
			break;

		// TODO other advisors
	}
}


static void buttonChangeAdvisor(int param1, int param2)
{
	if (param1) {
		UI_Advisors_setAdvisor(param1);
		UI_Window_requestRefresh();
	} else {
		UI_Window_goTo(Window_City);
	}
}

static void buttonHelp(int param1, int param2)
{
	if (currentAdvisor > 0 && currentAdvisor < 13) {
		UI_MessageDialog_show(advisorToMessageTextId[currentAdvisor], 1);
	}
}
