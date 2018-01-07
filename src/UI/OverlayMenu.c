#include "AllWindows.h"

#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/CityView.h"

#include "core/time.h"
#include "game/state.h"
#include "graphics/generic_button.h"
#include "graphics/panel.h"

static void handleSubmenu();

static void buttonMenuItem(int index, int param2);
static void buttonSubmenuItem(int index, int param2);

static generic_button menuCustomButtons[] = {
	{0, 0, 160, 24, GB_IMMEDIATE, buttonMenuItem, button_none, 0, 0},
	{0, 24, 160, 48, GB_IMMEDIATE, buttonMenuItem, button_none, 1, 0},
	{0, 48, 160, 72, GB_IMMEDIATE, buttonMenuItem, button_none, 2, 0},
	{0, 72, 160, 96, GB_IMMEDIATE, buttonMenuItem, button_none, 3, 0},
	{0, 96, 160, 120, GB_IMMEDIATE, buttonMenuItem, button_none, 4, 0},
	{0, 120, 160, 144, GB_IMMEDIATE, buttonMenuItem, button_none, 5, 0},
	{0, 144, 160, 168, GB_IMMEDIATE, buttonMenuItem, button_none, 6, 0},
	{0, 168, 160, 192, GB_IMMEDIATE, buttonMenuItem, button_none, 7, 0},
	{0, 192, 160, 216, GB_IMMEDIATE, buttonMenuItem, button_none, 8, 0},
	{0, 216, 160, 240, GB_IMMEDIATE, buttonMenuItem, button_none, 9, 0},
};
static generic_button submenuCustomButtons[] = {
	{0, 0, 160, 24, GB_IMMEDIATE, buttonSubmenuItem, button_none, 0, 0},
	{0, 24, 160, 48, GB_IMMEDIATE, buttonSubmenuItem, button_none, 1, 0},
	{0, 48, 160, 72, GB_IMMEDIATE, buttonSubmenuItem, button_none, 2, 0},
	{0, 72, 160, 96, GB_IMMEDIATE, buttonSubmenuItem, button_none, 3, 0},
	{0, 96, 160, 120, GB_IMMEDIATE, buttonSubmenuItem, button_none, 4, 0},
	{0, 120, 160, 144, GB_IMMEDIATE, buttonSubmenuItem, button_none, 5, 0},
	{0, 144, 160, 168, GB_IMMEDIATE, buttonSubmenuItem, button_none, 6, 0},
	{0, 168, 160, 192, GB_IMMEDIATE, buttonSubmenuItem, button_none, 7, 0},
	{0, 192, 160, 216, GB_IMMEDIATE, buttonSubmenuItem, button_none, 8, 0},
	{0, 216, 160, 240, GB_IMMEDIATE, buttonSubmenuItem, button_none, 9, 0},
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
static time_millis submenuFocusTime;

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
		label_draw(xOffset - 170, 74 + 24 * i,
			10, menuFocusButtonId == i + 1 ? 1 : 2);
		Widget_GameText_drawCentered(14, menuIdToOverlayId[i],
			xOffset - 170, 77 + 24 * i, 160, FONT_NORMAL_GREEN);
	}
	if (selectedSubmenu > 0) {
		Graphics_drawImage(image_group(GROUP_BULLET),
			xOffset - 185, 80 + 24 * selectedMenu);
		for (int i = 0; i < numSubmenuItems; i++) {
			label_draw(
				xOffset - 348, 74 + 24 * (i + selectedMenu),
				10, submenuFocusButtonId == i + 1 ? 1 : 2);
			Widget_GameText_drawCentered(14, submenuIdToOverlayId[selectedSubmenu][i],
				xOffset - 348, 77 + 24 * (i + selectedMenu), 160, FONT_NORMAL_GREEN);
		}
	}
}

void UI_OverlayMenu_handleMouse(const mouse *m)
{
	if (m->right.went_down) {
		UI_Window_goTo(Window_City);
		return;
	}
	generic_buttons_handle_mouse(m, Data_CityView.widthInPixels - 170, 72,
			menuCustomButtons, 8, &menuFocusButtonId);

	handleSubmenu();
	if (selectedSubmenu) {
		generic_buttons_handle_mouse(
			m, Data_CityView.widthInPixels - 348, 72 + 24 * selectedMenu,
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
		submenuFocusTime = time_get_millis();
		if (menuFocusButtonId) {
			selectedMenu = menuFocusButtonId - 1;
			selectedSubmenu = menuIdToSubmenuId[selectedMenu];
			numSubmenuItems = countSubmenuItems(selectedSubmenu);
		}
	} else if (time_get_millis() - submenuFocusTime > 500) {
		selectedSubmenu = 0;
		numSubmenuItems = 0;
	}
}

static void buttonMenuItem(int index, int param2)
{
	int overlay = menuIdToOverlayId[index];
	if (overlay == OVERLAY_NONE || overlay == OVERLAY_WATER || overlay == OVERLAY_RELIGION) {
		game_state_set_overlay(overlay);
	}
	UI_Window_goTo(Window_City);
}

static void buttonSubmenuItem(int index, int param2)
{
	int overlay = submenuIdToOverlayId[selectedSubmenu][index];
	if (overlay) {
		game_state_set_overlay(overlay);
	}
	UI_Window_goTo(Window_City);
}
