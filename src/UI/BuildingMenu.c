#include "AllWindows.h"

#include "Sidebar.h"
#include "Window.h"
#include "../Graphics.h"
#include "../SidebarMenu.h"
#include "../Widget.h"

#include "../Data/Building.h"
#include "../Data/CityView.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

#include "building/model.h"
#include "core/time.h"
#include "scenario/property.h"

static void drawMenuButtons();
static int handleBuildSubmenu();

static void buttonMenuIndex(int param1, int param2);
static void buttonMenuItem(int item);

static CustomButton buildMenuCustomButtons[] = {
	{0, 0, 256, 20, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 1, 0},
	{0, 24, 256, 44, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 2, 0},
	{0, 48, 256, 68, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 3, 0},
	{0, 72, 256, 92, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 4, 0},
	{0, 96, 256, 116, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 5, 0},
	{0, 120, 256, 140, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 6, 0},
	{0, 144, 256, 164, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 7, 0},
	{0, 168, 256, 188, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 8, 0},
	{0, 192, 256, 212, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 9, 0},
	{0, 216, 256, 236, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 10, 0},
	{0, 240, 256, 260, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 11, 0},
	{0, 264, 256, 284, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 12, 0},
	{0, 288, 256, 308, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 13, 0},
	{0, 312, 256, 332, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 14, 0},
	{0, 336, 256, 356, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 15, 0},
	{0, 360, 256, 380, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 16, 0},
	{0, 384, 256, 404, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 17, 0},
	{0, 408, 256, 428, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 18, 0},
	{0, 432, 256, 452, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 19, 0},
	{0, 456, 256, 476, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 20, 0},
	{0, 480, 256, 500, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 21, 0},
	{0, 504, 256, 524, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 22, 0},
	{0, 528, 256, 548, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 23, 0},
	{0, 552, 256, 572, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 24, 0},
	{0, 576, 256, 596, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 25, 0},
	{0, 600, 256, 620, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 26, 0},
	{0, 624, 256, 644, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 27, 0},
	{0, 648, 256, 668, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 28, 0},
	{0, 672, 256, 692, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 29, 0},
	{0, 696, 256, 716, CustomButton_Immediate, buttonMenuIndex, Widget_Button_doNothing, 30, 0},
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

void UI_BuildingMenu_drawSidebarImage(int xOffset, int forceDraw)
{
	if (Data_State.sidebarCollapsed && !forceDraw) {
		return;
	}
	if (!Data_State.selectedBuilding.type) {
		Graphics_drawImage(image_group(ID_Graphic_PanelWindows) + 12, xOffset, 239);
		return;
	}
	int graphicPanelWindows = image_group(ID_Graphic_PanelWindows);
	int graphicId;
	switch (menu.selectedSubmenu) {
		default:
		case 0:
			graphicId = graphicPanelWindows;
			break;
		case 1:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(ID_Graphic_PanelWindowsDesert);
			} else {
				graphicId = graphicPanelWindows + 11;
			}
			break;
		case 2:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(ID_Graphic_PanelWindowsDesert) + 1;
			} else {
				graphicId = graphicPanelWindows + 10;
			}
			break;
		case 3:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(ID_Graphic_PanelWindowsDesert) + 2;
			} else {
				graphicId = graphicPanelWindows + 3;
			}
			break;
		case 4:
			graphicId = graphicPanelWindows + 5;
			break;
		case 5:
			graphicId = graphicPanelWindows + 1;
			break;
		case 6:
			graphicId = graphicPanelWindows + 6;
			break;
		case 7:
			graphicId = graphicPanelWindows + 4;
			break;
		case 8:
			graphicId = graphicPanelWindows + 2;
			break;
		case 9:
			graphicId = graphicPanelWindows + 7;
			break;
		case 10:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(ID_Graphic_PanelWindowsDesert) + 3;
			} else {
				graphicId = graphicPanelWindows + 8;
			}
			break;
		case 11:
			graphicId = graphicPanelWindows + 9;
			break;
	}
	Graphics_drawImage(graphicId, xOffset, 239);
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
		Widget_Panel_drawSmallLabelButton(xOffset - 266, menu.yOffset + 110 + 24 * i,
			16, buildMenuFocusButtonId == i + 1 ? 1 : 2);
		int buildingType = SidebarMenu_getBuildingType(menu.selectedSubmenu, itemIndex);
		Widget_GameText_drawCentered(28, buildingType,
			xOffset - 266, menu.yOffset + 113 + 24 * i, 176, FONT_NORMAL_GREEN);
		if (buildingType == BUILDING_DRAGGABLE_RESERVOIR) {
			buildingType = BUILDING_RESERVOIR;
		}
		int cost = model_get_building(buildingType)->cost;
		if (buildingType == BUILDING_FORT) {
			cost = 0;
		}
		if (cost) {
			Widget_Text_drawMoney(cost,
				xOffset - 82, menu.yOffset + 114 + 24 * i, FONT_NORMAL_GREEN);
		}
	}
}

void UI_BuildingMenu_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_City);
		return;
	}

	if (!handleBuildSubmenu()) {
		UI_Sidebar_handleMouseBuildButtons(m);
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
	Data_State.selectedBuilding.roadLastUpdate = time_get_millis();
	Data_State.selectedBuilding.gridOffsetStart = 0;

	Data_Settings_Map.current.gridOffset = 0;
	
	Data_State.selectedBuilding.placementInProgress = 0;
	Data_State.selectedBuilding.xStart = 0;
	Data_State.selectedBuilding.xEnd = 0;
	Data_State.selectedBuilding.yStart = 0;
	Data_State.selectedBuilding.yEnd = 0;
	
	int type = Data_State.selectedBuilding.type = SidebarMenu_getBuildingType(menu.selectedSubmenu, item);
	if (type == BUILDING_MENU_FARMS || type == BUILDING_MENU_RAW_MATERIALS ||
		type == BUILDING_MENU_WORKSHOPS || type == BUILDING_FORT ||
		type == BUILDING_MENU_SMALL_TEMPLES || type == BUILDING_MENU_LARGE_TEMPLES) {
		switch (type) {
			case BUILDING_MENU_FARMS:
				menu.selectedSubmenu = 19;
				break;
			case BUILDING_MENU_RAW_MATERIALS:
				menu.selectedSubmenu = 20;
				break;
			case BUILDING_MENU_WORKSHOPS:
				menu.selectedSubmenu = 21;
				break;
			case BUILDING_MENU_SMALL_TEMPLES:
				menu.selectedSubmenu = 22;
				break;
			case BUILDING_MENU_LARGE_TEMPLES:
				menu.selectedSubmenu = 23;
				break;
			case BUILDING_FORT:
				menu.selectedSubmenu = 24;
				break;
		}
		menu.numItems = SidebarMenu_countBuildingMenuItems(menu.selectedSubmenu);
		menu.yOffset = yMenuOffsets[menu.numItems];
		Data_State.selectedBuilding.type = 0;
	} else {
		switch (type) {
			case BUILDING_WHEAT_FARM:
			case BUILDING_VEGETABLE_FARM:
			case BUILDING_FRUIT_FARM:
			case BUILDING_OLIVE_FARM:
			case BUILDING_VINES_FARM:
			case BUILDING_PIG_FARM:
				Data_State.selectedBuilding.meadowRequired = 1;
				break;
			case BUILDING_MARBLE_QUARRY:
			case BUILDING_IRON_MINE:
				Data_State.selectedBuilding.rockRequired = 1;
				break;
			case BUILDING_TIMBER_YARD:
				Data_State.selectedBuilding.treesRequired = 1;
				break;
			case BUILDING_CLAY_PIT:
				Data_State.selectedBuilding.waterRequired = 1;
				break;
			case BUILDING_GATEHOUSE:
			case BUILDING_TRIUMPHAL_ARCH:
				Data_State.selectedBuilding.roadRequired = 1;
				break;
			case BUILDING_TOWER:
				Data_State.selectedBuilding.wallRequired = 1;
				break;
		}
		UI_Window_goTo(Window_City);
	}
}
