#include "advisors_private.h"

#include "tooltip.h"
#include "window.h"
#include "cityinfo.h"

#include "core/calc.h"
#include "graphics/mouse.h"

static void arrowButtonWages(int param1, int param2);
static void buttonPriority(int param1, int param2);
static void buttonSetPriority(int param1, int param2);

static CustomButton categoryButtons[] = {
	{40, 77, 600, 99, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 0, 0},
	{40, 102, 600, 124, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 1, 0},
	{40, 127, 600, 149, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 2, 0},
	{40, 152, 600, 174, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 3, 0},
	{40, 177, 600, 199, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 4, 0},
	{40, 202, 600, 224, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 5, 0},
	{40, 227, 600, 249, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 6, 0},
	{40, 252, 600, 274, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 7, 0},
	{40, 277, 600, 299, CustomButton_Immediate, buttonPriority, Widget::Button::doNothing, 8, 0},
};

static ArrowButton wageButtons[] = {
	{158, 354, 17, 24, arrowButtonWages, 1, 0},
	{182, 354, 15, 24, arrowButtonWages, 0, 0}
};

static int focusButtonId;
static int arrowButtonFocus;

// labor priority stuff
static int prioritySelectedCategory;
static int priorityMaxItems;
static int priorityFocusButtonId;

static CustomButton priorityButtons[] = {
	{180, 256, 460, 281, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 0, 0}, // no prio
	{178, 221, 205, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 1, 0},
	{210, 221, 237, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 2, 0},
	{242, 221, 269, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 3, 0},
	{274, 221, 301, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 4, 0},
	{306, 221, 333, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 5, 0},
	{338, 221, 365, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 6, 0},
	{370, 221, 397, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 7, 0},
	{402, 221, 429, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 8, 0},
	{434, 221, 461, 248, CustomButton_Immediate, buttonSetPriority, Widget::Button::doNothing, 9, 0},
};

void UI_Advisor_Labor_drawBackground(int *advisorHeight)
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	*advisorHeight = 26;
	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, *advisorHeight);
	Graphics_drawImage(image_group(ID_Graphic_AdvisorIcons), baseOffsetX + 10, baseOffsetY + 10);
	
	Widget_GameText_draw(50, 0, baseOffsetX + 60, baseOffsetY + 12, FONT_LARGE_BLACK);
	
	// table headers
	Widget_GameText_draw(50, 21, baseOffsetX + 60, baseOffsetY + 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 22, baseOffsetX + 170, baseOffsetY + 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 23, baseOffsetX + 400, baseOffsetY + 56, FONT_SMALL_PLAIN);
	Widget_GameText_draw(50, 24, baseOffsetX + 500, baseOffsetY + 56, FONT_SMALL_PLAIN);
	
	int width = Widget::Text::drawNumber(Data_CityInfo.workersEmployed, '@', " ",
		baseOffsetX + 32, baseOffsetY + 320, FONT_NORMAL_BLACK
	);
	width += Widget_GameText_draw(50, 12,
		baseOffsetX + 32 + width, baseOffsetY + 320, FONT_NORMAL_BLACK
	);
	width += Widget::Text::drawNumber(Data_CityInfo.workersUnemployed, '@', " ",
		baseOffsetX + 50 + width, baseOffsetY + 320, FONT_NORMAL_BLACK
	);
	width += Widget_GameText_draw(50, 13,
		baseOffsetX + 50 + width, baseOffsetY + 320, FONT_NORMAL_BLACK
	);
	width += Widget::Text::drawNumber(Data_CityInfo.unemploymentPercentage, '@', "%)",
		baseOffsetX + 50 + width, baseOffsetY + 320, FONT_NORMAL_BLACK
	);
	
	Widget_Panel_drawInnerPanel(baseOffsetX + 64, baseOffsetY + 350, 32, 2);
	Widget_GameText_draw(50, 14, baseOffsetX + 70, baseOffsetY + 359, FONT_NORMAL_WHITE);
	
	width = Widget::Text::drawNumber(Data_CityInfo.wages, '@', " ",
		baseOffsetX + 230, baseOffsetY + 359, FONT_NORMAL_WHITE
	);
	width += Widget_GameText_draw(50, 15,
		baseOffsetX + 230 + width, baseOffsetY + 359, FONT_NORMAL_WHITE
	);
	width += Widget_GameText_draw(50, 18,
		baseOffsetX + 230 + width, baseOffsetY + 359, FONT_NORMAL_WHITE
	);
	width += Widget::Text::drawNumber(Data_CityInfo.wagesRome, '@', " )",
		baseOffsetX + 230 + width, baseOffsetY + 359, FONT_NORMAL_WHITE
	);
	
	width = Widget_GameText_draw(50, 19,
		baseOffsetX + 64, baseOffsetY + 390, FONT_NORMAL_BLACK
	);
	width += Widget::Text::drawMoney(Data_CityInfo.estimatedYearlyWages,
		baseOffsetX + 64 + width, baseOffsetY + 390, FONT_NORMAL_BLACK
	);
}

void UI_Advisor_Labor_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget::Button::drawArrowButtons(
		baseOffsetX, baseOffsetY, wageButtons, 2);

	Widget_Panel_drawInnerPanel(baseOffsetX + 32, baseOffsetY + 70, 36, 15);

	for (int i = 0; i < 9; i++) {
		int focus = i == focusButtonId - 1;
		Widget_Panel_drawButtonBorder(
			baseOffsetX + 40, baseOffsetY + 77 + 25 * i,
			560, 22, focus
		);
		if (Data_CityInfo.laborCategory[i].priority) {
			Graphics_drawImage(image_group(ID_Graphic_LaborPriorityLock),
				baseOffsetX + 70, baseOffsetY + 80 + 25 * i);
			Widget::Text::drawNumber(Data_CityInfo.laborCategory[i].priority, '@', " ",
				baseOffsetX + 90, baseOffsetY + 82 + 25 * i, FONT_NORMAL_WHITE);
		}
		Widget_GameText_draw(50, i + 1,
			baseOffsetX + 170, baseOffsetY + 82 + 25 * i, FONT_NORMAL_WHITE);
		Widget::Text::drawNumber(Data_CityInfo.laborCategory[i].workersNeeded, '@', " ",
			baseOffsetX + 410, baseOffsetY + 82 + 25 * i, FONT_NORMAL_WHITE);
		font_t font = FONT_NORMAL_WHITE;
		if (Data_CityInfo.laborCategory[i].workersNeeded != Data_CityInfo.laborCategory[i].workersAllocated) {
			font = FONT_NORMAL_RED;
		}
		Widget::Text::drawNumber(Data_CityInfo.laborCategory[i].workersAllocated, '@', " ",
			baseOffsetX + 510, baseOffsetY + 82 + 25 * i, font);
	}
}

void UI_Advisor_Labor_handleMouse()
{
	int offsetX = Data_Screen.offset640x480.x;
	int offsetY = Data_Screen.offset640x480.y;
	if (!Widget::Button::handleCustomButtons(offsetX, offsetY, categoryButtons, 9, &focusButtonId)) {
		arrowButtonFocus = Widget::Button::handleArrowButtons(offsetX, offsetY, wageButtons, 2);
	}
}

static void arrowButtonWages(int isDown, int param2)
{
	if (isDown == 1) {
		Data_CityInfo.wages--;
	} else if (isDown == 0) {
		Data_CityInfo.wages++;
	}
	Data_CityInfo.wages = calc_bound(Data_CityInfo.wages, 0, 100);
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
	Widget_GameText_drawCentered(50, 25, baseOffsetX + 160, baseOffsetY + 185, 320, FONT_LARGE_BLACK);
	for (int i = 0; i < 9; i++) {
		Graphics_drawRect(baseOffsetX + 178 + 32 * i, baseOffsetY + 221, 27, 27, COLOR_BLACK);
		Widget_GameText_drawCentered(50, 27 + i, baseOffsetX + 178 + 32 * i, baseOffsetY + 224, 27, FONT_LARGE_BLACK);
		if (i >= priorityMaxItems) {
			Graphics_shadeRect(baseOffsetX + 179 + 32 * i, baseOffsetY + 222, 25, 25, 1);
		}
	}

	Graphics_drawRect(baseOffsetX + 180, baseOffsetY + 256, 280, 25, COLOR_BLACK);
	Widget_GameText_drawCentered(50, 26, baseOffsetX + 180, baseOffsetY + 263, 280, FONT_NORMAL_BLACK);
	Widget_GameText_drawCentered(13, 3, baseOffsetX + 160, baseOffsetY + 296, 320, FONT_NORMAL_BLACK);
}

void UI_LaborPriorityDialog_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	color_t color;
	for (int i = 0; i < 9; i++) {
		color = COLOR_BLACK;
		if (i == priorityFocusButtonId - 2) {
			color = COLOR_RED;
		}
		Graphics_drawRect(baseOffsetX + 178 + 32 * i, baseOffsetY + 221, 27, 27, color);
	}
	color = COLOR_BLACK;
	if (priorityFocusButtonId == 1) {
		color = COLOR_RED;
	}
	Graphics_drawRect(baseOffsetX + 180, baseOffsetY + 256, 280, 25, color);
}

void UI_LaborPriorityDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_Advisors);
	} else {
		int offsetX = Data_Screen.offset640x480.x;
		int offsetY = Data_Screen.offset640x480.y;
		Widget::Button::handleCustomButtons(offsetX, offsetY, priorityButtons, 1 + priorityMaxItems, &priorityFocusButtonId);
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
