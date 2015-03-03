#include "AllWindows.h"

#include "Window.h"
#include "../Graphics.h"
#include "../Time.h"
#include "../Widget.h"

#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Mouse.h"
#include "../Data/State.h"

static void handleSubmenu();

static void buttonMenuItem(int param1, int param2);
static void buttonSubmenuItem(int param1, int param2);

static CustomButton menuCustomButtons[] = {
	{0, 0, 160, 24, buttonMenuItem, Widget_Button_doNothing, 1, 0, 0},
	{0, 24, 160, 48, buttonMenuItem, Widget_Button_doNothing, 1, 1, 0},
	{0, 48, 160, 72, buttonMenuItem, Widget_Button_doNothing, 1, 2, 0},
	{0, 72, 160, 96, buttonMenuItem, Widget_Button_doNothing, 1, 3, 0},
	{0, 96, 160, 120, buttonMenuItem, Widget_Button_doNothing, 1, 4, 0},
	{0, 120, 160, 144, buttonMenuItem, Widget_Button_doNothing, 1, 5, 0},
	{0, 144, 160, 168, buttonMenuItem, Widget_Button_doNothing, 1, 6, 0},
	{0, 168, 160, 192, buttonMenuItem, Widget_Button_doNothing, 1, 7, 0},
	{0, 192, 160, 216, buttonMenuItem, Widget_Button_doNothing, 1, 8, 0},
	{0, 216, 160, 240, buttonMenuItem, Widget_Button_doNothing, 1, 9, 0},
};
static CustomButton submenuCustomButtons[] = {
	{0, 0, 160, 24, buttonSubmenuItem, Widget_Button_doNothing, 1, 0, 0},
	{0, 24, 160, 48, buttonSubmenuItem, Widget_Button_doNothing, 1, 1, 0},
	{0, 48, 160, 72, buttonSubmenuItem, Widget_Button_doNothing, 1, 2, 0},
	{0, 72, 160, 96, buttonSubmenuItem, Widget_Button_doNothing, 1, 3, 0},
	{0, 96, 160, 120, buttonSubmenuItem, Widget_Button_doNothing, 1, 4, 0},
	{0, 120, 160, 144, buttonSubmenuItem, Widget_Button_doNothing, 1, 5, 0},
	{0, 144, 160, 168, buttonSubmenuItem, Widget_Button_doNothing, 1, 6, 0},
	{0, 168, 160, 192, buttonSubmenuItem, Widget_Button_doNothing, 1, 7, 0},
	{0, 192, 160, 216, buttonSubmenuItem, Widget_Button_doNothing, 1, 8, 0},
	{0, 216, 160, 240, buttonSubmenuItem, Widget_Button_doNothing, 1, 9, 0},
};

static int menuIdToOverlayId[8] = {0, 2, 1, 3, 5, 6, 7, 4};
static int menuIdToSubmenuId[8] = {0, 0, 1, 2, 3, 4, 5, 0};

static int submenuIdToOverlayId[6][8] = {
	{0},
	{8, 9, 10, 28, 29, 0},
	{11, 12, 13, 14, 15, 0},
	{16, 17, 18, 19, 0},
	{20, 21, 22, 23, 0},
	{24, 25, 26},
};

static int selectedMenu;
static int selectedSubmenu;
static int numSubmenuItems;
static TimeMillis submenuFocusTime;

static int menuFocusButtonId;
static int submenuFocusButtonId;

void UI_OverlayMenu_init()
{
	selectedSubmenu = 0;
	numSubmenuItems = 0;
}

void UI_OverlayMenu_drawBackground()
{
	UI_City_drawBackground();
}

void UI_OverlayMenu_drawForeground()
{
	UI_City_drawCity();
	int xOffset = Data_CityView.widthInPixels;
	for (int i = 0; i < 8; i++) {
		Widget_Panel_drawSmallLabelButton(xOffset - 170, 74 + 24 * i,
			10, menuFocusButtonId == i + 1 ? 1 : 2);
		Widget_GameText_drawCentered(14, menuIdToOverlayId[i],
			xOffset - 170, 77 + 24 * i, 160, Font_NormalGreen);
	}
	if (selectedSubmenu > 0) {
		Graphics_drawImage(GraphicId(ID_Graphic_Bullet),
			xOffset - 185, 80 + 24 * selectedMenu);
		for (int i = 0; i < numSubmenuItems; i++) {
			Widget_Panel_drawSmallLabelButton(
				xOffset - 348, 74 + 24 * (i + selectedMenu),
				10, submenuFocusButtonId == i + 1 ? 1 : 2);
			Widget_GameText_drawCentered(14, submenuIdToOverlayId[selectedSubmenu][i],
				xOffset - 348, 77 + 24 * (i + selectedMenu), 160, Font_NormalGreen);
		}
	}
}

void UI_OverlayMenu_handleMouse()
{
	if (Data_Mouse.right.wentDown) {
		UI_Window_goTo(Window_City);
		return;
	}
	Widget_Button_handleCustomButtons(Data_CityView.widthInPixels - 170, 72,
			menuCustomButtons, 8, &menuFocusButtonId);

	handleSubmenu();
	if (selectedSubmenu) {
		Widget_Button_handleCustomButtons(
			Data_CityView.widthInPixels - 348, 72 + 24 * selectedMenu,
			submenuCustomButtons, numSubmenuItems, &submenuFocusButtonId);
	}
}

static int countSubmenuItems(int submenuId)
{
	int total = 0;
	for (int i = 0; i < 8 && submenuIdToOverlayId[selectedSubmenu][i] > 0; i++) {
		total++;
	}
	return total;
}

static void handleSubmenu()
{
	if (menuFocusButtonId || submenuFocusButtonId) {
		submenuFocusTime = Time_getMillis();
		if (menuFocusButtonId) {
			selectedMenu = menuFocusButtonId - 1;
			selectedSubmenu = menuIdToSubmenuId[selectedMenu];
			numSubmenuItems = countSubmenuItems(selectedSubmenu);
		}
	} else if (Time_getMillis() - submenuFocusTime > 500) {
		selectedSubmenu = 0;
		numSubmenuItems = 0;
	}
}

static void buttonMenuItem(int index, int param2)
{
	int overlay = menuIdToOverlayId[index];
	if (!overlay) {
		Data_State.previousOverlay = Data_State.currentOverlay;
		Data_State.currentOverlay = 0;
	} else if (overlay == Overlay_Water || overlay == Overlay_Religion) {
		Data_State.previousOverlay = 0;
		Data_State.currentOverlay = overlay;
	}
	UI_Window_goTo(Window_City);
}

static void buttonSubmenuItem(int index, int param2)
{
	int overlay = submenuIdToOverlayId[selectedSubmenu][index];
	if (overlay) {
		if (Data_State.currentOverlay) {
			Data_State.previousOverlay = 0;
		} else {
			Data_State.previousOverlay = Data_State.currentOverlay;
		}
		Data_State.currentOverlay = overlay;
	}
	UI_Window_goTo(Window_City);
}
