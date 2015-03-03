#include "Advisors_private.h"

#include "Tooltip.h"
#include "Window.h"
#include "../CityInfo.h"
#include "../Data/Mouse.h"
#include "../Util.h"

static void arrowButtonWages(int param1, int param2);
static void buttonPriority(int param1, int param2);
static void buttonSetPriority(int param1, int param2);

static CustomButton categoryButtons[9] = {
	{40, 77, 600, 99, buttonPriority, Widget_Button_doNothing, 1, 0, 0},
	{40, 102, 600, 124, buttonPriority, Widget_Button_doNothing, 1, 1, 0},
	{40, 127, 600, 149, buttonPriority, Widget_Button_doNothing, 1, 2, 0},
	{40, 152, 600, 174, buttonPriority, Widget_Button_doNothing, 1, 3, 0},
	{40, 177, 600, 199, buttonPriority, Widget_Button_doNothing, 1, 4, 0},
	{40, 202, 600, 224, buttonPriority, Widget_Button_doNothing, 1, 5, 0},
	{40, 227, 600, 249, buttonPriority, Widget_Button_doNothing, 1, 6, 0},
	{40, 252, 600, 274, buttonPriority, Widget_Button_doNothing, 1, 7, 0},
	{40, 277, 600, 299, buttonPriority, Widget_Button_doNothing, 1, 8, 0},
};

static ArrowButton wageButtons[2] = {
	{158, 354, 17, 24, arrowButtonWages, 1, 0},
	{182, 354, 15, 24, arrowButtonWages, 0, 0}
};

static int focusButtonId;
static int arrowButtonFocus;

// labor priority stuff
static int prioritySelectedCategory;
static int priorityMaxItems;
static int priorityFocusButtonId;

static CustomButton priorityButtons[10] = {
	{180, 256, 460, 281, buttonSetPriority, Widget_Button_doNothing, 1, 0, 0}, // no prio
	{178, 221, 205, 248, buttonSetPriority, Widget_Button_doNothing, 1, 1, 0},
	{210, 221, 237, 248, buttonSetPriority, Widget_Button_doNothing, 1, 2, 0},
	{242, 221, 269, 248, buttonSetPriority, Widget_Button_doNothing, 1, 3, 0},
	{274, 221, 301, 248, buttonSetPriority, Widget_Button_doNothing, 1, 4, 0},
	{306, 221, 333, 248, buttonSetPriority, Widget_Button_doNothing, 1, 5, 0},
	{338, 221, 365, 248, buttonSetPriority, Widget_Button_doNothing, 1, 6, 0},
	{370, 221, 397, 248, buttonSetPriority, Widget_Button_doNothing, 1, 7, 0},
	{402, 221, 429, 248, buttonSetPriority, Widget_Button_doNothing, 1, 8, 0},
	{434, 221, 461, 248, buttonSetPriority, Widget_Button_doNothing, 1, 9, 0},
};

void UI_Advisor_Labor_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 26;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons), baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(50, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack);
	
	// table headers
	Widget_GameText_draw(50, 21, baseOffsetX + 60, baseOffsetY + 56, Font_SmallPlain);
	Widget_GameText_draw(50, 22, baseOffsetX + 170, baseOffsetY + 56, Font_SmallPlain);
	Widget_GameText_draw(50, 23, baseOffsetX + 400, baseOffsetY + 56, Font_SmallPlain);
	Widget_GameText_draw(50, 24, baseOffsetX + 500, baseOffsetY + 56, Font_SmallPlain);
	
	int width = Widget_Text_drawNumber(Data_CityInfo.workersEmployed, '@', " ",
		baseOffsetX + 32, baseOffsetY + 320, Font_NormalBlack
	);
	width += Widget_GameText_draw(50, 12,
		baseOffsetX + 32 + width, baseOffsetY + 320, Font_NormalBlack
	);
	width += Widget_Text_drawNumber(Data_CityInfo.workersUnemployed, '@', " ",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack
	);
	width += Widget_GameText_draw(50, 13,
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack
	);
	width += Widget_Text_drawNumber(Data_CityInfo.unemploymentPercentage, '@', "%)",
		baseOffsetX + 50 + width, baseOffsetY + 320, Font_NormalBlack
	);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 350, 32, 2);
	Widget_GameText_draw(50, 14, baseOffsetX + 70, baseOffsetY + 359, Font_NormalWhite);
	
	width = Widget_Text_drawNumber(Data_CityInfo.wages, '@', " ",
		baseOffsetX + 230, baseOffsetY + 359, Font_NormalWhite
	);
	width += Widget_GameText_draw(50, 15,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite
	);
	width += Widget_GameText_draw(50, 18,
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite
	);
	width += Widget_Text_drawNumber(Data_CityInfo.wagesRome, '@', " )",
		baseOffsetX + 230 + width, baseOffsetY + 359, Font_NormalWhite
	);
	
	width = Widget_GameText_draw(50, 19,
		baseOffsetX + 64, baseOffsetY + 390, Font_NormalBlack
	);
	width += Widget_Text_drawNumber(Data_CityInfo.estimatedYearlyWages, '@', " Dn",
		baseOffsetX + 64 + width, baseOffsetY + 390, Font_NormalBlack
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
		int focus = i == focusButtonId - 1;
		Widget_Panel_drawButtonBorder(
			baseOffsetX + 40, baseOffsetY + 77 + 25 * i,
			560, 22, focus
		);
		if (Data_CityInfo.laborCategory[i].priority) {
			Graphics_drawImage(GraphicId(ID_Graphic_LaborPriorityLock),
				baseOffsetX + 70, baseOffsetY + 80 + 25 * i);
			Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].priority, '@', " ",
				baseOffsetX + 90, baseOffsetY + 82 + 25 * i, Font_NormalWhite);
		}
		Widget_GameText_draw(50, i + 1,
			baseOffsetX + 170, baseOffsetY + 82 + 25 * i, Font_NormalWhite);
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersNeeded, '@', " ",
			baseOffsetX + 410, baseOffsetY + 82 + 25 * i, Font_NormalWhite);
		Font font = Font_NormalWhite;
		if (Data_CityInfo.laborCategory[i].workersNeeded != Data_CityInfo.laborCategory[i].workersAllocated) {
			font = Font_NormalRed;
		}
		Widget_Text_drawNumber(Data_CityInfo.laborCategory[i].workersAllocated, '@', " ",
			baseOffsetX + 510, baseOffsetY + 82 + 25 * i, font);
	}
}

void UI_Advisor_Labor_handleMouse()
{
	int offsetX = Data_Screen.offset640x480.x;
	int offsetY = Data_Screen.offset640x480.y;
	if (!Widget_Button_handleCustomButtons(offsetX, offsetY, categoryButtons, 9, &focusButtonId)) {
		arrowButtonFocus = Widget_Button_handleArrowButtons(offsetX, offsetY, wageButtons, 2);
	}
}

static void arrowButtonWages(int isDown, int param2)
{
	if (isDown == 1) {
		Data_CityInfo.wages--;
	} else if (isDown == 0) {
		Data_CityInfo.wages++;
	}
	BOUND(Data_CityInfo.wages, 0, 100);
	CityInfo_Finance_calculateEstimatedWages();
	CityInfo_Finance_calculateTotals();
	UI_Window_requestRefresh();
}

static void buttonPriority(int category, int param2)
{
	prioritySelectedCategory = category;
	priorityMaxItems = 0;
	for (int i = 0; i < 9; i++) {
		if (Data_CityInfo.laborCategory[i].priority > 0) {
			++priorityMaxItems;
		}
	}
	if (priorityMaxItems < 9 && !Data_CityInfo.laborCategory[prioritySelectedCategory].priority) {
		// allow space for new priority...
		++priorityMaxItems;
	}
	UI_Window_goTo(Window_LaborPriorityDialog);
}

int UI_Advisor_Labor_getTooltip()
{
	if (focusButtonId) {
		return 90;
	} else if (arrowButtonFocus) {
		return 91;
	} else {
		return 0;
	}
}

void UI_LaborPriorityDialog_drawBackground()
{
	int dummy;
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	UI_Advisor_Labor_drawBackground(&dummy);
	UI_Advisor_Labor_drawForeground();

	Widget_Panel_drawOuterPanel(baseOffsetX + 160, baseOffsetY + 176, 20, 9);
	Widget_GameText_drawCentered(50, 25, baseOffsetX + 160, baseOffsetY + 185, 320, Font_LargeBlack);
	for (int i = 0; i < 9; i++) {
		Graphics_drawRect(baseOffsetX + 178 + 32 * i, baseOffsetY + 221, 27, 27, Color_Black);
		Widget_GameText_drawCentered(50, 27 + i, baseOffsetX + 178 + 32 * i, baseOffsetY + 224, 27, Font_LargeBlack);
		if (i >= priorityMaxItems) {
			Graphics_shadeRect(baseOffsetX + 179 + 32 * i, baseOffsetY + 222, 25, 25, 1);
		}
	}

	Graphics_drawRect(baseOffsetX + 180, baseOffsetY + 256, 280, 25, Color_Black);
	Widget_GameText_drawCentered(50, 26, baseOffsetX + 180, baseOffsetY + 263, 280, Font_NormalBlack);
	Widget_GameText_drawCentered(13, 3, baseOffsetX + 160, baseOffsetY + 296, 320, Font_NormalBlack);
}

void UI_LaborPriorityDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Color color;
	for (int i = 0; i < 9; i++) {
		color = Color_Black;
		if (i == priorityFocusButtonId - 2) {
			color = Color_Red;
		}
		Graphics_drawRect(baseOffsetX + 178 + 32 * i, baseOffsetY + 221, 27, 27, color);
	}
	color = Color_Black;
	if (priorityFocusButtonId == 1) {
		color = Color_Red;
	}
	Graphics_drawRect(baseOffsetX + 180, baseOffsetY + 256, 280, 25, color);
}

void UI_LaborPriorityDialog_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
		UI_Window_goTo(Window_Advisors);
	} else {
		int offsetX = Data_Screen.offset640x480.x;
		int offsetY = Data_Screen.offset640x480.y;
		Widget_Button_handleCustomButtons(offsetX, offsetY, priorityButtons, 1 + priorityMaxItems, &priorityFocusButtonId);
	}
}

static void buttonSetPriority(int newPriority, int param2)
{
	int oldPriority = Data_CityInfo.laborCategory[prioritySelectedCategory].priority;
	if (oldPriority != newPriority) {
		int shift;
		int fromPrio;
		int toPrio;
		if (!oldPriority && newPriority) {
			// shift all bigger than 'newPriority' by one down (+1)
			shift = 1;
			fromPrio = newPriority;
			toPrio = 9;
		} else if (oldPriority && !newPriority) {
			// shift all bigger than 'oldPriority' by one up (-1)
			shift = -1;
			fromPrio = oldPriority;
			toPrio = 9;
		} else if (newPriority < oldPriority) {
			// shift all between new and old by one down (+1)
			shift = 1;
			fromPrio = newPriority;
			toPrio = oldPriority;
		} else {
			// shift all between old and new by one up (-1)
			shift = -1;
			fromPrio = oldPriority;
			toPrio = newPriority;
		}
		Data_CityInfo.laborCategory[prioritySelectedCategory].priority = newPriority;
		for (int i = 0; i < 9; i++) {
			if (i == prioritySelectedCategory) {
				continue;
			}
			int curPrio = Data_CityInfo.laborCategory[i].priority;
			if (fromPrio <= curPrio && curPrio <= toPrio) {
				Data_CityInfo.laborCategory[i].priority += shift;
			}
		}
		CityInfo_Labor_allocateWorkersToCategories();
		CityInfo_Labor_allocateWorkersToBuildings();
	}
	UI_Window_goTo(Window_Advisors);
}

void UI_LaborPriorityDialog_getTooltip(struct TooltipContext *c)
{
	if (!priorityFocusButtonId) {
		return;
	}
	c->type = TooltipType_Button;
	if (priorityFocusButtonId == 1) {
		c->textId = 92;
	} else {
		c->textId = 93;
	}
}
