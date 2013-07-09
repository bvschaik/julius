#include "Advisors_private.h"
#include "Window.h"
#include "../Data/Scenario.h"

static void drawHistoryGraph(int fullSize, int x, int y);
static void drawCensusGraph(int fullSize, int x, int y);
static void drawSocietyGraph(int fullSize, int x, int y);
static void getYAxis(int maxValue, int *yMax, int *yShift);
static int getPopulationAtMonth(int max, int month);
static void getMinMaxMonthYear(int maxMonths, int *startMonth, int *startYear, int *endMonth, int *endYear);
static void buttonGraph(int param1, int param2);

static CustomButton graphButtons[2] = {
	{503,  61, 607, 116, buttonGraph, Widget_Button_doNothing, 1, 0, 0},
	{503, 161, 607, 216, buttonGraph, Widget_Button_doNothing, 1, 1, 0},
};

static int focusButtonId;

void UI_Advisor_Population_drawBackground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX, baseOffsetY, 40, 27);
	Graphics_drawImage(GraphicId(ID_Graphic_AdvisorIcons) + 5, baseOffsetX + 10, baseOffsetY + 10);
	
	//Widget_GameText_draw(50, 0, baseOffsetX + 60, baseOffsetY + 12, Font_LargeBlack, 0);

	Graphics_drawImage(GraphicId(ID_Graphic_PanelWindows) + 14, baseOffsetX + 56, baseOffsetY + 60);
	
	int bigText, topText, botText;
	void (*bigGraph)(int, int, int);
	void (*topGraph)(int, int, int);
	void (*botGraph)(int, int, int);
	switch (Data_CityInfo_Extra.populationGraphOrder) {
		default:
		case 0:
			bigText = 6;
			topText = 4;
			botText = 5;
			bigGraph = drawHistoryGraph;
			topGraph = drawCensusGraph;
			botGraph = drawSocietyGraph;
			break;
		case 1:
			bigText = 6;
			topText = 5;
			botText = 4;
			bigGraph = drawHistoryGraph;
			topGraph = drawSocietyGraph;
			botGraph = drawCensusGraph;
			break;
		case 2:
			bigText = 7;
			topText = 3;
			botText = 5;
			bigGraph = drawCensusGraph;
			topGraph = drawHistoryGraph;
			botGraph = drawSocietyGraph;
			break;
		case 3:
			bigText = 7;
			topText = 5;
			botText = 3;
			bigGraph = drawCensusGraph;
			topGraph = drawSocietyGraph;
			botGraph = drawHistoryGraph;
			break;
		case 4:
			bigText = 8;
			topText = 3;
			botText = 4;
			bigGraph = drawSocietyGraph;
			topGraph = drawHistoryGraph;
			botGraph = drawCensusGraph;
			break;
		case 5:
			bigText = 8;
			topText = 4;
			botText = 3;
			bigGraph = drawSocietyGraph;
			topGraph = drawCensusGraph;
			botGraph = drawHistoryGraph;
			break;
	}
	Widget_GameText_drawCentered(55, bigText,
		baseOffsetX + 60, baseOffsetY + 295, 400, Font_NormalBlack, 0);
	Widget_GameText_drawCentered(55, topText,
		baseOffsetX + 504, baseOffsetY + 120, 100, Font_NormalBlack, 0);
	Widget_GameText_drawCentered(55, botText,
		baseOffsetX + 504, baseOffsetY + 220, 100, Font_NormalBlack, 0);
	bigGraph(1, baseOffsetX + 64, baseOffsetY + 64);
	topGraph(0, baseOffsetX + 505, baseOffsetY + 63);
	botGraph(0, baseOffsetX + 505, baseOffsetY + 163);

	// TODO other population info
}

void UI_Advisor_Population_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	if (focusButtonId == 0) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 60, 106, 57, 0);
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 160, 106, 57, 0);
	} else if (focusButtonId == 1) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 60, 106, 57, 1);
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 160, 106, 57, 0);
	} else if (focusButtonId == 2) {
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 60, 106, 57, 0);
		Widget_Panel_drawButtonBorder(baseOffsetX + 501, baseOffsetY + 160, 106, 57, 1);
	}
}

void UI_Advisor_Population_handleMouse()
{
	Widget_Button_handleCustomButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		graphButtons, 2, &focusButtonId);
}

static void drawHistoryGraph(int fullSize, int x, int y)
{
	int maxMonths;
	if (Data_CityInfo.monthsSinceStart <= 20) {
		maxMonths = 20;
	} else if (Data_CityInfo.monthsSinceStart <= 40) {
		maxMonths = 40;
	} else if (Data_CityInfo.monthsSinceStart <= 100) {
		maxMonths = 100;
	} else if (Data_CityInfo.monthsSinceStart <= 200) {
		maxMonths = 200;
	} else {
		maxMonths = 400;
	}
	if (!fullSize) {
		if (maxMonths <= 40) {
			maxMonths = 20;
		} else {
			maxMonths = 100;
		}
	}
	// determine max value
	int maxValue = 0;
	for (int m = 0; m < maxMonths; m++) {
		int value = getPopulationAtMonth(maxMonths, m);
		if (value > maxValue) {
			maxValue = value;
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		Widget_Text_drawNumberCentered(yMax, '@', " ",
			x - 66, y - 3, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(yMax / 2, '@', " ",
			x - 66, y + 96, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(0, '@', " ",
			x - 66, y + 196, 60, Font_SmallPlain, 0);
		// x axis
		int startMonth, startYear, endMonth, endYear;
		getMinMaxMonthYear(maxMonths, &startMonth, &startYear, &endMonth, &endYear);

		int width = Widget_GameText_draw(25, startMonth, x - 20, y + 210, Font_SmallPlain, 0);
		Widget_GameText_drawYear(startYear, x + width - 20, y + 210, Font_SmallPlain, 0);

		width = Widget_GameText_draw(25, endMonth, x + 380, y + 210, Font_SmallPlain, 0);
		Widget_GameText_drawYear(startYear, x + width + 380, y + 210, Font_SmallPlain, 0);
	}

	if (fullSize) {
		Graphics_setClipRectangle(0, 0, Data_Screen.width, y + 200);
		for (int m = 0; m < maxMonths; m++) {
			int pop = getPopulationAtMonth(maxMonths, m);
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				switch (maxMonths) {
					case 20:
						Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar),
							x + 20 * m, y + 200 - val);
						break;
					case 40:
						Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar) + 1,
							x + 10 * m, y + 200 - val);
						break;
					case 100:
						Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar) + 2,
							x + 4 * m, y + 200 - val);
						break;
					case 200:
						Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar) + 3,
							x + 2 * m, y + 200 - val);
						break;
					default:
						Graphics_drawLine(x + m, y + 200 - val,
							x + m, y + 199, Color_Red);
						break;
				}
			}
		}
		Graphics_resetClipRectangle();
	} else {
		yShift += 2;
		for (int m = 0; m < maxMonths; m++) {
			int val = getPopulationAtMonth(maxMonths, m) >> yShift;
			if (val > 0) {
				if (maxMonths == 20) {
					Graphics_fillRect(x + m, y + 50 - val, 4, val + 1, Color_Red);
				} else {
					Graphics_drawLine(x + m, y + 50 - val, x + m, y + 50, Color_Red);
				}
			}
		}
	}
}

static void drawCensusGraph(int fullSize, int x, int y)
{
	int maxValue = 0;
	for (int i = 0; i < 100; i++) {
		if (Data_CityInfo.populationPerAge[i] > maxValue) {
			maxValue = Data_CityInfo.populationPerAge[i];
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		Widget_Text_drawNumberCentered(yMax, '@', " ",
			x - 66, y - 3, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(yMax / 2, '@', " ",
			x - 66, y + 96, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(0, '@', " ",
			x - 66, y + 196, 60, Font_SmallPlain, 0);
		// x axis
		for (int i = 0; i <= 10; i++) {
			Widget_Text_drawNumberCentered(i * 10, '@', " ",
				x + 40 * i - 22, y + 210, 40, Font_SmallPlain, 0);
		}
	}

	if (fullSize) {
		Graphics_setClipRectangle(0, 0, Data_Screen.width, y + 200);
		for (int i = 0; i < 100; i++) {
			int pop = Data_CityInfo.populationPerAge[i];
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar) + 2,
					x + 4 * i, y + 200 - val);
			}
		}
		Graphics_resetClipRectangle();
	} else {
		yShift += 2;
		for (int i = 0; i < 100; i++) {
			int val = Data_CityInfo.populationPerAge[i] >> yShift;
			if (val > 0) {
				Graphics_drawLine(x + i, y + 50 - val, x + i, y + 50, Color_Red);
			}
		}
	}
}

static void drawSocietyGraph(int fullSize, int x, int y)
{
	int maxValue = 0;
	for (int i = 0; i < 20; i++) {
		if (Data_CityInfo.populationPerLevel[i] > maxValue) {
			maxValue = Data_CityInfo.populationPerLevel[i];
		}
	}
	int yMax, yShift;
	getYAxis(maxValue, &yMax, &yShift);
	if (fullSize) {
		// y axis
		Widget_Text_drawNumberCentered(yMax, '@', " ",
			x - 66, y - 3, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(yMax / 2, '@', " ",
			x - 66, y + 96, 60, Font_SmallPlain, 0);
		Widget_Text_drawNumberCentered(0, '@', " ",
			x - 66, y + 196, 60, Font_SmallPlain, 0);
		// x axis
		Widget_GameText_drawCentered(55, 9,
			x - 80, y + 210, 200, Font_SmallPlain, 0);
		Widget_GameText_drawCentered(55, 10,
			x + 280, y + 210, 200, Font_SmallPlain, 0);
	}

	if (fullSize) {
		Graphics_setClipRectangle(0, 0, Data_Screen.width, y + 200);
		for (int i = 0; i < 20; i++) {
			int pop = Data_CityInfo.populationPerLevel[i];
			int val;
			if (yShift == -1) {
				val = 2 * pop;
			} else {
				val = pop >> yShift;
			}
			if (val > 0) {
				Graphics_drawImage(GraphicId(ID_Graphic_PopulationGraphBar),
					x + 20 * i, y + 200 - val);
			}
		}
		Graphics_resetClipRectangle();
	} else {
		yShift += 2;
		for (int i = 0; i < 20; i++) {
			int val = Data_CityInfo.populationPerLevel[i] >> yShift;
			if (val > 0) {
				Graphics_fillRect(x + 5 * i, y + 50 - val, 4, val + 1, Color_Red);
			}
		}
	}
}

static void getYAxis(int maxValue, int *yMax, int *yShift)
{
	if (maxValue <= 100) {
		*yMax = 100;
		*yShift = -1;
	} else if (maxValue <= 200) {
		*yMax = 200;
		*yShift = 0;
	} else if (maxValue <= 400) {
		*yMax = 400;
		*yShift = 1;
	} else if (maxValue <= 800) {
		*yMax = 800;
		*yShift = 2;
	} else if (maxValue <= 1600) {
		*yMax = 1600;
		*yShift = 3;
	} else if (maxValue <= 3200) {
		*yMax = 3200;
		*yShift = 4;
	} else if (maxValue <= 6400) {
		*yMax = 6400;
		*yShift = 5;
	} else if (maxValue <= 12800) {
		*yMax = 12800;
		*yShift = 6;
	} else if (maxValue <= 25600) {
		*yMax = 25600;
		*yShift = 7;
	} else {
		*yMax = 51200;
		*yShift = 8;
	}
}

static int getPopulationAtMonth(int max, int month)
{
	int startOffset = 0;
	if (Data_CityInfo.monthsSinceStart > max) {
		startOffset = Data_CityInfo.monthsSinceStart + 2400 - max;
	}
	int index = (startOffset + month) % 2400;
	return Data_CityInfo.monthlyPopulation[index];
}

static void getMinMaxMonthYear(int maxMonths, int *startMonth, int *startYear, int *endMonth, int *endYear)
{
	if (Data_CityInfo.monthsSinceStart > maxMonths) {
		*endMonth = Data_CityInfo_Extra.gameTimeMonth - 1;
		*endYear = Data_CityInfo_Extra.gameTimeYear;
		if (*endMonth < 0) {
			*endMonth += 12;
			*endYear -= 1;
		}
		*startMonth = 11 - (maxMonths % 12);
		*startYear = *endYear - maxMonths / 12;
	} else {
		*startMonth = 0;
		*startYear = Data_Scenario.startYear;
		*endMonth = (maxMonths + *startMonth) % 12;
		*endYear = (maxMonths + *startMonth) / 12 + *startYear;
	}
}

static void buttonGraph(int param1, int param2)
{
	int newOrder;
	switch (Data_CityInfo_Extra.populationGraphOrder) {
		default:
		case 0:
			newOrder = param1 ? 5 : 2;
			break;
		case 1:
			newOrder = param1 ? 3 : 4;
			break;
		case 2:
			newOrder = param1 ? 4 : 0;
			break;
		case 3:
			newOrder = param1 ? 1 : 5;
			break;
		case 4:
			newOrder = param1 ? 2 : 1;
			break;
		case 5:
			newOrder = param1 ? 0 : 3;
			break;
	}
	Data_CityInfo_Extra.populationGraphOrder = newOrder;
	UI_Window_requestRefresh();
}

