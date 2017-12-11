#include "Advisors_private.h"

#include "AllWindows.h"
#include "Window.h"
#include "MessageDialog.h"

#include "../CityInfo.h"
#include "../Formation.h"

#include "../Data/Constants.h"

#include "city/culture.h"
#include "city/warning.h"
#include "game/settings.h"
#include "game/tutorial.h"

static void buttonChangeAdvisor(int param1, int param2);
static void buttonHelp(int param1, int param2);

static ImageButton helpButton = {
	11, -7, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget_Button_doNothing, 0, 0, 1
};

static CustomButton advisorButtons[] = {
	{12, 1, 52, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 1, 0},
	{60, 1, 100, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 2, 0},
	{108, 1, 148, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 3, 0},
	{156, 1, 196, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 4, 0},
	{204, 1, 244, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 5, 0},
	{252, 1, 292, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 6, 0},
	{300, 1, 340, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 7, 0},
	{348, 1, 388, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 8, 0},
	{396, 1, 436, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 9, 0},
	{444, 1, 484, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 10, 0},
	{492, 1, 532, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 11, 0},
	{540, 1, 580, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 12, 0},
	{588, 1, 624, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 0, 0},
};

static const int advisorToMessageTextId[] = {
	0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static int currentAdvisor = Advisor_None;

static int focusButtonId;
static int advisorHeight;

int UI_Advisors_getId()
{
	return currentAdvisor;
}

void UI_Advisors_goToFromMessage(int advisor)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == NOT_AVAILABLE || avail == NOT_AVAILABLE_YET) {
        if (UI_Window_getId() == Window_MessageDialog) {
            UI_MessageDialog_close();
            UI_Window_goTo(Window_City);
        }
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
        return;
    }
	currentAdvisor = advisor;
	setting_set_last_advisor(advisor);
	UI_Advisors_init();
	UI_Window_goTo(Window_Advisors);
}

void UI_Advisors_goToFromSidepanel()
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == AVAILABLE) {
        currentAdvisor = setting_last_advisor();
        UI_Advisors_init();
        UI_Window_goTo(Window_Advisors);
    } else {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
    }
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
	city_culture_update_coverage();

	CityInfo_Resource_calculateFoodAndSupplyRomeWheat();
	Formation_calculateFigures();

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
	Graphics_drawFullScreenImage(image_group(GROUP_ADVISOR_BACKGROUND));
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	Graphics_drawImage(image_group(GROUP_PANEL_WINDOWS) + 13,
		baseOffsetX, baseOffsetY + 432
	);

	for (int i = 0; i < 13; i++) {
		int selectedOffset = 0;
		if (currentAdvisor && i == currentAdvisor - 1) {
			selectedOffset = 13;
		}
		Graphics_drawImage(image_group(GROUP_ADVISOR_ICONS) + i + selectedOffset,
			baseOffsetX + 48 * i + 12, baseOffsetY + 441
		);
	}
}

void UI_Advisors_handleMouse(const mouse *m)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	if (Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY + 440, advisorButtons, 13, &focusButtonId)) {
		return;
	}
	int buttonId;
	Widget_Button_handleImageButtons(baseOffsetX, baseOffsetY + 16 * (advisorHeight - 2), &helpButton, 1, &buttonId);
	if (buttonId) {
		focusButtonId = -1;
		return;
	}
	if (m->right.went_up) {
		UI_Window_goTo(Window_City);
		return;
	}

	switch (currentAdvisor) {
		case Advisor_Labor:
			UI_Advisor_Labor_handleMouse();
			break;
		case Advisor_Military:
			UI_Advisor_Military_handleMouse();
			break;
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
	}
}

static void buttonChangeAdvisor(int param1, int param2)
{
	if (param1) {
		currentAdvisor = param1;
		setting_set_last_advisor(param1);
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

void UI_Advisors_getTooltip(struct TooltipContext *c)
{
	if (focusButtonId) {
		c->type = TooltipType_Button;
		if (focusButtonId == -1) {
			c->textId = 1; // help button
		} else {
			c->textId = 69 + focusButtonId;
		}
		return;
	}
	int textId = 0;
	switch (currentAdvisor) {
		case Advisor_Labor:
			textId = UI_Advisor_Labor_getTooltip();
			break;
		case Advisor_Imperial:
			textId = UI_Advisor_Imperial_getTooltip();
			break;
		case Advisor_Ratings:
			textId = UI_Advisor_Ratings_getTooltip();
			break;
		case Advisor_Trade:
			textId = UI_Advisor_Trade_getTooltip();
			break;
		case Advisor_Population:
			textId = UI_Advisor_Population_getTooltip();
			break;
		case Advisor_Entertainment:
			textId = UI_Advisor_Entertainment_getTooltip();
			break;
		case Advisor_Financial:
			textId = UI_Advisor_Financial_getTooltip();
			break;
	}
	if (textId) {
		c->textId = textId;
		c->type = TooltipType_Button;
	}
}
