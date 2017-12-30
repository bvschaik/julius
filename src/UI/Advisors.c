#include "Advisors_private.h"

#include "AllWindows.h"
#include "MessageDialog.h"

#include "../CityInfo.h"
#include "../Formation.h"

#include "city/constants.h"
#include "city/culture.h"
#include "city/labor.h"
#include "city/resource.h"
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

static advisor_type currentAdvisor = ADVISOR_NONE;

static int focusButtonId;
static int advisorHeight;

int UI_Advisors_getId()
{
	return currentAdvisor;
}

void UI_Advisors_goToFromMessage(advisor_type advisor)
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
	city_labor_allocate_workers();

	CityInfo_Finance_calculateEstimatedTaxes();
	CityInfo_Finance_calculateEstimatedWages();
	CityInfo_Finance_updateInterest();
	CityInfo_Finance_updateSalary();
	CityInfo_Finance_calculateTotals();

	city_culture_calculate_demands();
	city_culture_update_coverage();

	city_resource_calculate_food_stocks_and_supply_wheat();
	Formation_calculateFigures();

	CityInfo_Ratings_updateCultureExplanation();
	CityInfo_Ratings_updateProsperityExplanation();
	CityInfo_Ratings_updatePeaceExplanation();
	CityInfo_Ratings_updateFavorExplanation();

	if (currentAdvisor == ADVISOR_ENTERTAINMENT) {
        UI_Advisor_Entertainment_init();
	}
}

void UI_Advisors_drawBackground()
{
	UI_Advisor_drawGeneralBackground();
	switch (currentAdvisor) {
		case ADVISOR_LABOR:
			UI_Advisor_Labor_drawBackground(&advisorHeight);
			break;
		case ADVISOR_MILITARY:
			UI_Advisor_Military_drawBackground(&advisorHeight);
			break;
		case ADVISOR_IMPERIAL:
			UI_Advisor_Imperial_drawBackground(&advisorHeight);
			break;
		case ADVISOR_RATINGS:
			UI_Advisor_Ratings_drawBackground(&advisorHeight);
			break;
		case ADVISOR_TRADE:
			UI_Advisor_Trade_drawBackground(&advisorHeight);
			break;
		case ADVISOR_POPULATION:
			UI_Advisor_Population_drawBackground(&advisorHeight);
			break;
		case ADVISOR_HEALTH:
			UI_Advisor_Health_drawBackground(&advisorHeight);
			break;
		case ADVISOR_EDUCATION:
			UI_Advisor_Education_drawBackground(&advisorHeight);
			break;
		case ADVISOR_ENTERTAINMENT:
			UI_Advisor_Entertainment_drawBackground(&advisorHeight);
			break;
		case ADVISOR_RELIGION:
			UI_Advisor_Religion_drawBackground(&advisorHeight);
			break;
		case ADVISOR_FINANCIAL:
			UI_Advisor_Financial_drawBackground(&advisorHeight);
			break;
		case ADVISOR_CHIEF:
			UI_Advisor_Chief_drawBackground(&advisorHeight);
			break;
        default:
            break;
	}
}

void UI_Advisors_drawForeground()
{
	Widget_Button_drawImageButtons(Data_Screen.offset640x480.x,
		Data_Screen.offset640x480.y + 16 * (advisorHeight - 2),
		&helpButton, 1);

	switch (currentAdvisor) {
		case ADVISOR_LABOR:
			UI_Advisor_Labor_drawForeground();
			break;
		case ADVISOR_MILITARY:
			UI_Advisor_Military_drawForeground();
			break;
		case ADVISOR_IMPERIAL:
			UI_Advisor_Imperial_drawForeground();
			break;
		case ADVISOR_RATINGS:
			UI_Advisor_Ratings_drawForeground();
			break;
		case ADVISOR_TRADE:
			UI_Advisor_Trade_drawForeground();
			break;
		case ADVISOR_POPULATION:
			UI_Advisor_Population_drawForeground();
			break;
		case ADVISOR_ENTERTAINMENT:
			UI_Advisor_Entertainment_drawForeground();
			break;
		case ADVISOR_FINANCIAL:
			UI_Advisor_Financial_drawForeground();
			break;
        default:
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
		case ADVISOR_LABOR:
			UI_Advisor_Labor_handleMouse();
			break;
		case ADVISOR_MILITARY:
			UI_Advisor_Military_handleMouse();
			break;
		case ADVISOR_IMPERIAL:
			UI_Advisor_Imperial_handleMouse();
			break;
		case ADVISOR_RATINGS:
			UI_Advisor_Ratings_handleMouse();
			break;
		case ADVISOR_TRADE:
			UI_Advisor_Trade_handleMouse();
			break;
		case ADVISOR_POPULATION:
			UI_Advisor_Population_handleMouse();
			break;
		case ADVISOR_ENTERTAINMENT:
			UI_Advisor_Entertainment_handleMouse();
			break;
		case ADVISOR_FINANCIAL:
			UI_Advisor_Financial_handleMouse();
			break;
        default:
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
		case ADVISOR_LABOR:
			textId = UI_Advisor_Labor_getTooltip();
			break;
		case ADVISOR_IMPERIAL:
			textId = UI_Advisor_Imperial_getTooltip();
			break;
		case ADVISOR_RATINGS:
			textId = UI_Advisor_Ratings_getTooltip();
			break;
		case ADVISOR_TRADE:
			textId = UI_Advisor_Trade_getTooltip();
			break;
		case ADVISOR_POPULATION:
			textId = UI_Advisor_Population_getTooltip();
			break;
		case ADVISOR_ENTERTAINMENT:
			textId = UI_Advisor_Entertainment_getTooltip();
			break;
		case ADVISOR_FINANCIAL:
			textId = UI_Advisor_Financial_getTooltip();
			break;
        default:
            break;
	}
	if (textId) {
		c->textId = textId;
		c->type = TooltipType_Button;
	}
}
