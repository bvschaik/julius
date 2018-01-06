#include "Advisors_private.h"

#include "AllWindows.h"
#include "MessageDialog.h"

#include "city/constants.h"
#include "city/culture.h"
#include "city/finance.h"
#include "city/labor.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "city/warning.h"
#include "figure/formation.h"
#include "game/settings.h"
#include "game/tutorial.h"

static void buttonChangeAdvisor(int param1, int param2);
static void buttonHelp(int param1, int param2);

static ImageButton helpButton = {
	11, -7, 27, 27, ImageButton_Normal, 134, 0, buttonHelp, Widget_Button_doNothing, 0, 0, 1
};

static CustomButton advisorButtons[] = {
	{12, 1, 52, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_LABOR, 0},
	{60, 1, 100, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_MILITARY, 0},
	{108, 1, 148, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_IMPERIAL, 0},
	{156, 1, 196, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_RATINGS, 0},
	{204, 1, 244, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_TRADE, 0},
	{252, 1, 292, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_POPULATION, 0},
	{300, 1, 340, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_HEALTH, 0},
	{348, 1, 388, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_EDUCATION, 0},
	{396, 1, 436, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_ENTERTAINMENT, 0},
	{444, 1, 484, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_RELIGION, 0},
	{492, 1, 532, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_FINANCIAL, 0},
	{540, 1, 580, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, ADVISOR_CHIEF, 0},
	{588, 1, 624, 41, CustomButton_Immediate, buttonChangeAdvisor, Widget_Button_doNothing, 0, 0},
};

static struct {
    void (*draw_background)(int *height);
    void (*draw_foreground)();
    void (*handle_mouse)();
    int (*get_tooltip)();
} windows[] = {
    {0, 0, 0, 0},
    {UI_Advisor_Labor_drawBackground, UI_Advisor_Labor_drawForeground, UI_Advisor_Labor_handleMouse, UI_Advisor_Labor_getTooltip},
    {UI_Advisor_Military_drawBackground, UI_Advisor_Military_drawForeground, UI_Advisor_Military_handleMouse, 0},
    {UI_Advisor_Imperial_drawBackground, UI_Advisor_Imperial_drawForeground, UI_Advisor_Imperial_handleMouse, UI_Advisor_Imperial_getTooltip},
    {UI_Advisor_Ratings_drawBackground, UI_Advisor_Ratings_drawForeground, UI_Advisor_Ratings_handleMouse, UI_Advisor_Ratings_getTooltip},
    {UI_Advisor_Trade_drawBackground, UI_Advisor_Trade_drawForeground, UI_Advisor_Trade_handleMouse, UI_Advisor_Trade_getTooltip},
    {UI_Advisor_Population_drawBackground, UI_Advisor_Population_drawForeground, UI_Advisor_Population_handleMouse, UI_Advisor_Population_getTooltip},
    {UI_Advisor_Health_drawBackground, 0, 0, 0},
    {UI_Advisor_Education_drawBackground, 0, 0, 0},
    {UI_Advisor_Entertainment_drawBackground, UI_Advisor_Entertainment_drawForeground, UI_Advisor_Entertainment_handleMouse, UI_Advisor_Entertainment_getTooltip},
    {UI_Advisor_Religion_drawBackground, 0, 0, 0},
    {UI_Advisor_Financial_drawBackground, UI_Advisor_Financial_drawForeground, UI_Advisor_Financial_handleMouse, UI_Advisor_Financial_getTooltip},
    {UI_Advisor_Chief_drawBackground, 0, 0, 0},
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

	city_finance_estimate_taxes();
	city_finance_estimate_wages();
	city_finance_update_interest();
	city_finance_update_salary();
	city_finance_calculate_totals();

	city_culture_calculate_demands();
	city_culture_update_coverage();

	city_resource_calculate_food_stocks_and_supply_wheat();
	formation_calculate_figures();

	city_ratings_update_explanations();

	if (currentAdvisor == ADVISOR_ENTERTAINMENT) {
        UI_Advisor_Entertainment_init();
	}
}

void UI_Advisors_drawBackground()
{
	UI_Advisor_drawGeneralBackground();
	windows[currentAdvisor].draw_background(&advisorHeight);
}

void UI_Advisors_drawForeground()
{
	Widget_Button_drawImageButtons(Data_Screen.offset640x480.x,
		Data_Screen.offset640x480.y + 16 * (advisorHeight - 2),
		&helpButton, 1);

    if (windows[currentAdvisor].draw_foreground) {
        windows[currentAdvisor].draw_foreground();
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

    if (windows[currentAdvisor].handle_mouse) {
        windows[currentAdvisor].handle_mouse();
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
    if (windows[currentAdvisor].get_tooltip) {
        textId = windows[currentAdvisor].get_tooltip();
    }
	if (textId) {
		c->textId = textId;
		c->type = TooltipType_Button;
	}
}
