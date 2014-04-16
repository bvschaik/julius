#include "AllWindows.h"

#include "Sidebar.h"
#include "Window.h"
#include "../Graphics.h"
#include "../SidebarMenu.h"
#include "../Time.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Model.h"
#include "../Data/Mouse.h"
#include "../Data/State.h"

static void drawMenuButtons();
static int handleBuildSubmenu();

static void buttonMenuIndex(int param1, int param2);
static void buttonMenuItem(int item);

static CustomButton buildMenuCustomButtons[] = {
	{0, 0, 256, 20, buttonMenuIndex, Widget_Button_doNothing, 1, 1, 0},
	{0, 24, 256, 44, buttonMenuIndex, Widget_Button_doNothing, 1, 2, 0},
	{0, 48, 256, 68, buttonMenuIndex, Widget_Button_doNothing, 1, 3, 0},
	{0, 72, 256, 92, buttonMenuIndex, Widget_Button_doNothing, 1, 4, 0},
	{0, 96, 256, 116, buttonMenuIndex, Widget_Button_doNothing, 1, 5, 0},
	{0, 120, 256, 140, buttonMenuIndex, Widget_Button_doNothing, 1, 6, 0},
	{0, 144, 256, 164, buttonMenuIndex, Widget_Button_doNothing, 1, 7, 0},
	{0, 168, 256, 188, buttonMenuIndex, Widget_Button_doNothing, 1, 8, 0},
	{0, 192, 256, 212, buttonMenuIndex, Widget_Button_doNothing, 1, 9, 0},
	{0, 216, 256, 236, buttonMenuIndex, Widget_Button_doNothing, 1, 10, 0},
	{0, 240, 256, 260, buttonMenuIndex, Widget_Button_doNothing, 1, 11, 0},
	{0, 264, 256, 284, buttonMenuIndex, Widget_Button_doNothing, 1, 12, 0},
	{0, 288, 256, 308, buttonMenuIndex, Widget_Button_doNothing, 1, 13, 0},
	{0, 312, 256, 332, buttonMenuIndex, Widget_Button_doNothing, 1, 14, 0},
	{0, 336, 256, 356, buttonMenuIndex, Widget_Button_doNothing, 1, 15, 0},
	{0, 360, 256, 380, buttonMenuIndex, Widget_Button_doNothing, 1, 16, 0},
	{0, 384, 256, 404, buttonMenuIndex, Widget_Button_doNothing, 1, 17, 0},
	{0, 408, 256, 428, buttonMenuIndex, Widget_Button_doNothing, 1, 18, 0},
	{0, 432, 256, 452, buttonMenuIndex, Widget_Button_doNothing, 1, 19, 0},
	{0, 456, 256, 476, buttonMenuIndex, Widget_Button_doNothing, 1, 20, 0},
	{0, 480, 256, 500, buttonMenuIndex, Widget_Button_doNothing, 1, 21, 0},
	{0, 504, 256, 524, buttonMenuIndex, Widget_Button_doNothing, 1, 22, 0},
	{0, 528, 256, 548, buttonMenuIndex, Widget_Button_doNothing, 1, 23, 0},
	{0, 552, 256, 572, buttonMenuIndex, Widget_Button_doNothing, 1, 24, 0},
	{0, 576, 256, 596, buttonMenuIndex, Widget_Button_doNothing, 1, 25, 0},
	{0, 600, 256, 620, buttonMenuIndex, Widget_Button_doNothing, 1, 26, 0},
	{0, 624, 256, 644, buttonMenuIndex, Widget_Button_doNothing, 1, 27, 0},
	{0, 648, 256, 668, buttonMenuIndex, Widget_Button_doNothing, 1, 28, 0},
	{0, 672, 256, 692, buttonMenuIndex, Widget_Button_doNothing, 1, 29, 0},
	{0, 696, 256, 716, buttonMenuIndex, Widget_Button_doNothing, 1, 30, 0},
};

static const int yMenuOffsets[24] = {
	0, 322, 306, 274, 258, 226, 210, 178, 162, 130, 114,
	82, 66, 34, 18, -30, -46, -62, -78, -78, -94,
	-94, -110, -110
};
static int buildMenuFocusButtonId;

static struct {
	int selectedSubmenu;
	int numItems;
	int xOffset;
	int yOffset;
} menu;

void UI_BuildingMenu_init(int submenu)
{
	menu.selectedSubmenu = submenu;
	menu.numItems = SidebarMenu_countBuildingMenuItems(submenu);
	menu.yOffset = yMenuOffsets[menu.numItems];
	if (submenu == 0 || submenu == 1 || submenu == 2) {
		buttonMenuItem(0);
	} else {
		UI_Window_goTo(Window_BuildingMenu);
	}
}

void UI_BuildingMenu_drawBackground()
{
	UI_City_drawBackground();
	UI_City_drawCity();
}

void UI_BuildingMenu_drawForeground()
{
	UI_City_drawCity();
	drawMenuButtons();
}

static void drawMenuButtons()
{
	int xOffset = Data_CityView.widthInPixels;
	int itemIndex = -1;
	for (int i = 0; i < menu.numItems; i++) {
		itemIndex = SidebarMenu_getNextBuildingItemIndex(menu.selectedSubmenu, itemIndex);
		int textOffset = 0;
		int isSubmenu = 1;
		switch (menu.selectedSubmenu) {
			case 14: textOffset = -1; break;
			case 15: textOffset = 4; break;
			case 16: textOffset = 6; break;
			case 17: textOffset = 9; break;
			case 18: textOffset = 17; break;
			case 19: textOffset = 19; break;
			case 26: textOffset = 22; break;
			default: isSubmenu = 0; break;
		}
		if (isSubmenu) {
			Widget_Panel_drawSmallLabelButton(6, xOffset - 170, menu.yOffset + 110 + 24 * i,
				10, 1, buildMenuFocusButtonId == i + 1 ? 1 : 2);
			Widget_GameText_drawCentered(48,
				SidebarMenu_getBuildingType(menu.selectedSubmenu, itemIndex),
				xOffset - 170, menu.yOffset + 113 +  + 24 * i, 160, Font_NormalGreen);
		} else {
			Widget_Panel_drawSmallLabelButton(6, xOffset - 266, menu.yOffset + 110 + 24 * i,
				16, 1, buildMenuFocusButtonId == i + 1 ? 1 : 2);
			int buildingType = SidebarMenu_getBuildingType(menu.selectedSubmenu, itemIndex);
			Widget_GameText_drawCentered(28, buildingType,
				xOffset - 266, menu.yOffset + 113 +  + 24 * i, 176, Font_NormalGreen);
			if (buildingType == Building_DraggableReservoir) {
				buildingType = Building_Reservoir;
			}
			int cost = Data_Model_Buildings[buildingType].cost;
			if (buildingType == Building_FortGround__) {
				cost = 0;
			}
			if (cost) {
				Widget_Text_drawNumber(cost, '@', "Dn",
					xOffset - 82, menu.yOffset + 114 + 24 * i, Font_NormalGreen);
			}
		}
	}
}

void UI_BuildingMenu_handleMouse()
{
	if (Data_Mouse.right.wentDown) {
		UI_Window_goTo(Window_City);
		return;
	}

	if (!handleBuildSubmenu()) {
		UI_Sidebar_handleMouseBuildButtons();
	}
}

static int handleBuildSubmenu()
{
	return Widget_Button_handleCustomButtons(
		Data_CityView.widthInPixels - 258, menu.yOffset + 110,
		buildMenuCustomButtons, menu.numItems, &buildMenuFocusButtonId);
}

static int buttonIndexToSubmenuItem(int index)
{
	int item = -1;
	for (int i = 0; i <= index; i++) {
		item = SidebarMenu_getNextBuildingItemIndex(menu.selectedSubmenu, item);
	}
	return item;
}

static void buttonMenuIndex(int param1, int param2)
{
	buttonMenuItem(buttonIndexToSubmenuItem(param1 - 1));
}

static void buttonMenuItem(int item)
{
	Data_State.selectedBuilding.wallRequired = 0;
	Data_State.selectedBuilding.waterRequired = 0;
	Data_State.selectedBuilding.treesRequired = 0;
	Data_State.selectedBuilding.rockRequired = 0;
	Data_State.selectedBuilding.meadowRequired = 0;
	Data_State.selectedBuilding.roadRequired = 0;
	Data_State.selectedBuilding.roadLastUpdate = Time_getMillis();
	Data_State.selectedBuilding.gridOffset = 0;
	
	Data_State.selectedBuilding.isDragging = 0;
	Data_State.selectedBuilding.x = 0;
	Data_State.selectedBuilding.xStart = 0;
	Data_State.selectedBuilding.y = 0;
	Data_State.selectedBuilding.yStart = 0;
	
	int type = Data_State.selectedBuilding.type = SidebarMenu_getBuildingType(menu.selectedSubmenu, item);
	if (type == Building_Menu_Farms || type == Building_Menu_RawMaterials ||
		type == Building_Menu_Workshops || type == Building_FortGround__ ||
		type == Building_Menu_SmallTemples || type == Building_Menu_LargeTemples) {
		switch (type) {
			case Building_Menu_Farms:
				menu.selectedSubmenu = 19;
				break;
			case Building_Menu_RawMaterials:
				menu.selectedSubmenu = 20;
				break;
			case Building_Menu_Workshops:
				menu.selectedSubmenu = 21;
				break;
			case Building_Menu_SmallTemples:
				menu.selectedSubmenu = 22;
				break;
			case Building_Menu_LargeTemples:
				menu.selectedSubmenu = 23;
				break;
			case Building_FortGround__:
				menu.selectedSubmenu = 24;
				break;
		}
		menu.numItems = SidebarMenu_countBuildingMenuItems(menu.selectedSubmenu);
		menu.yOffset = yMenuOffsets[menu.numItems];
		Data_State.selectedBuilding.type = 0;
	} else {
		switch (type) {
			case Building_WheatFarm:
			case Building_VegetableFarm:
			case Building_FruitFarm:
			case Building_OliveFarm:
			case Building_VinesFarm:
			case Building_PigFarm:
				Data_State.selectedBuilding.meadowRequired = 1;
				break;
			case Building_MarbleQuarry:
			case Building_IronMine:
				Data_State.selectedBuilding.rockRequired = 1;
				break;
			case Building_TimberYard:
				Data_State.selectedBuilding.treesRequired = 1;
				break;
			case Building_ClayPit:
				Data_State.selectedBuilding.waterRequired = 1;
				break;
			case Building_Gatehouse:
			case Building_TriumphalArch:
				Data_State.selectedBuilding.roadRequired = 1;
				break;
			case Building_Tower:
				Data_State.selectedBuilding.wallRequired = 1;
				break;
		}
		UI_Window_goTo(Window_City);
	}
}
