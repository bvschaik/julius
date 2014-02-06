#include "AllWindows.h"
#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../CityInfoUpdater.h"
#include "Advisors_private.h"
#include "../Data/CityInfo.h"
#include "../Data/Screen.h"
#include "../Data/Mouse.h"

static void drawButtons();
static void buttonGod(int param1, int param2);
static void buttonSize(int param1, int param2);
static void buttonHelp(int param1, int param2);
static void buttonClose(int param1, int param2);
static void buttonHoldFestival(int param1, int param2);

static ImageButton imageButtonsBottom[] = {
	{58, 316, 27, 27, 4, 134, 0, buttonHelp, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{558, 319, 24, 24, 4, 134, 4, buttonClose, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{358, 317, 34, 34, 4, 96, 0, buttonHoldFestival, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0},
	{400, 317, 34, 34, 4, 96, 4, buttonClose, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
};

static CustomButton buttonsGodsSize[] = {
	{}
/*46h, 60h, 96h, 0BAh, \
.data:005ED680                                         ; DATA XREF: fun_handleMouseClick+2403o
.data:005ED680                                         ; fun_handleMouseClick+2425o
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectGod, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 0, 0>
.data:005ED680                 C3CustomButton <0AAh, 60h, 0FAh, 0BAh, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectGod, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 1, 0>
.data:005ED680                 C3CustomButton <10Eh, 60h, 15Eh, 0BAh, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectGod, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 2, 0>
.data:005ED680                 C3CustomButton <172h, 60h, 1C2h, 0BAh, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectGod, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 3, 0>
.data:005ED680                 C3CustomButton <1D6h, 60h, 226h, 0BAh, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectGod, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 4, 0>
.data:005ED680                 C3CustomButton <66h, 0D8h, 214h, 0F2h, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectSize, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 1, 0>
.data:005ED680                 C3CustomButton <66h, 0F6h, 214h, 110h, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectSize, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 2, 0>
.data:005ED680                 C3CustomButton <66h, 114h, 214h, 12Eh, \
.data:005ED680                                 offset j_fun_dialogHoldFestival_selectSize, \
.data:005ED680                                 offset j_fun_clickDoNothing, 0, 0, 0, 1, 3, 0
*/
};

static int focusButton;

void UI_HoldFestivalDialog_drawBackground()
{
	UI_Advisor_drawGeneralBackground();

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	Widget_Panel_drawOuterPanel(baseOffsetX + 48, baseOffsetY + 48, 34, 20);
	Widget_GameText_drawCentered(58, 25 + Data_CityInfo.festivalGod,
		baseOffsetX + 48, baseOffsetY + 60, 544, Font_LargeBlack);
	for (int god = 0; god < 5; god++) {
		if (god == Data_CityInfo.festivalGod) {
			Widget_Panel_drawButtonBorder(
				baseOffsetX + 100 * god + 66,
				baseOffsetY + 92, 90, 100, 1);
			Graphics_drawImage(GraphicId(ID_Graphic_PanelWindows) + god + 21,
				baseOffsetX + 100 * god + 70, baseOffsetY + 96);
		} else {
			Graphics_drawImage(GraphicId(ID_Graphic_PanelWindows) + god + 16,
				baseOffsetX + 100 * god + 70, baseOffsetY + 96);
		}
	}
	drawButtons();
	Widget_GameText_draw(58, 30 + Data_CityInfo.festivalSize,
		baseOffsetX + 180, baseOffsetY + 322, Font_NormalBlack);
}

void UI_HoldFestivalDialog_drawForeground()
{
	drawButtons();
	Widget_Button_drawImageButtons(
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y,
		imageButtonsBottom, 4);
}

static void drawButtons()
{
}

void UI_HoldFestivalDialog_handleMouse()
{
	if (Data_Mouse.isRightClick) {
		UI_Window_goTo(Window_Advisors);
		return;
	}

	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;

	if (!Widget_Button_handleImageButtons(baseOffsetX, baseOffsetY, imageButtonsBottom, 4)) {
		Widget_Button_handleCustomButtons(baseOffsetX, baseOffsetY, buttonsGodsSize, 8, &focusButton);
	}
}

static void buttonGod(int god, int param2)
{
	Data_CityInfo.festivalGod = god;
	UI_Window_requestRefresh();
}

static void buttonSize(int size, int param2)
{
	if (Data_CityInfo.treasury > -5000) {
		if (size != 3 || !Data_CityInfo.festivalNoWineAvailable) {
			Data_CityInfo.festivalSize = size;
			UI_Window_requestRefresh();
		}
	}
}

static void buttonHelp(int param1, int param2)
{
	// TODO showHelpDialog(28, 1);
}

static void buttonClose(int param1, int param2)
{
	UI_Window_goTo(Window_Advisors);
}

static void buttonHoldFestival(int param1, int param2)
{
	if (Data_CityInfo.treasury <= -5000) {
		return;
	}
	Data_CityInfo.plannedFestivalGod = Data_CityInfo.festivalGod;
	Data_CityInfo.plannedFestivalSize = Data_CityInfo.plannedFestivalSize;
	int cost;
	if (Data_CityInfo.festivalSize == 1) {
		Data_CityInfo.plannedFestivalMonthsToGo = 2;
		cost = Data_CityInfo.festivalCostSmall;
	} else if (Data_CityInfo.festivalSize == 2) {
		Data_CityInfo.plannedFestivalMonthsToGo = 3;
		cost = Data_CityInfo.festivalCostLarge;
	} else {
		Data_CityInfo.plannedFestivalMonthsToGo = 4;
		cost = Data_CityInfo.festivalCostGrand;
	}

	Data_CityInfo.treasury -= cost;
	Data_CityInfo.financeSundriesThisYear += cost;

	if (Data_CityInfo.festivalSize == 3) {
		// TODO removeGoodsFromStorage(Resource_Wine, Data_CityInfo.festivalGrandWine);
	}
	UI_Window_goTo(Window_Advisors);
}

