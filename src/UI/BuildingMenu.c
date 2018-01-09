#include "AllWindows.h"

#include "Sidebar.h"

#include "../Data/CityView.h"
#include "../Data/State.h"

#include "building/construction.h"
#include "building/menu.h"
#include "building/model.h"
#include "city/view.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/property.h"

static void drawMenuButtons();
static int handleBuildSubmenu(const mouse *m);

static void buttonMenuIndex(int param1, int param2);
static void buttonMenuItem(int item);

static generic_button buildMenuCustomButtons[] = {
	{0, 0, 256, 20, GB_IMMEDIATE, buttonMenuIndex, button_none, 1, 0},
	{0, 24, 256, 44, GB_IMMEDIATE, buttonMenuIndex, button_none, 2, 0},
	{0, 48, 256, 68, GB_IMMEDIATE, buttonMenuIndex, button_none, 3, 0},
	{0, 72, 256, 92, GB_IMMEDIATE, buttonMenuIndex, button_none, 4, 0},
	{0, 96, 256, 116, GB_IMMEDIATE, buttonMenuIndex, button_none, 5, 0},
	{0, 120, 256, 140, GB_IMMEDIATE, buttonMenuIndex, button_none, 6, 0},
	{0, 144, 256, 164, GB_IMMEDIATE, buttonMenuIndex, button_none, 7, 0},
	{0, 168, 256, 188, GB_IMMEDIATE, buttonMenuIndex, button_none, 8, 0},
	{0, 192, 256, 212, GB_IMMEDIATE, buttonMenuIndex, button_none, 9, 0},
	{0, 216, 256, 236, GB_IMMEDIATE, buttonMenuIndex, button_none, 10, 0},
	{0, 240, 256, 260, GB_IMMEDIATE, buttonMenuIndex, button_none, 11, 0},
	{0, 264, 256, 284, GB_IMMEDIATE, buttonMenuIndex, button_none, 12, 0},
	{0, 288, 256, 308, GB_IMMEDIATE, buttonMenuIndex, button_none, 13, 0},
	{0, 312, 256, 332, GB_IMMEDIATE, buttonMenuIndex, button_none, 14, 0},
	{0, 336, 256, 356, GB_IMMEDIATE, buttonMenuIndex, button_none, 15, 0},
	{0, 360, 256, 380, GB_IMMEDIATE, buttonMenuIndex, button_none, 16, 0},
	{0, 384, 256, 404, GB_IMMEDIATE, buttonMenuIndex, button_none, 17, 0},
	{0, 408, 256, 428, GB_IMMEDIATE, buttonMenuIndex, button_none, 18, 0},
	{0, 432, 256, 452, GB_IMMEDIATE, buttonMenuIndex, button_none, 19, 0},
	{0, 456, 256, 476, GB_IMMEDIATE, buttonMenuIndex, button_none, 20, 0},
	{0, 480, 256, 500, GB_IMMEDIATE, buttonMenuIndex, button_none, 21, 0},
	{0, 504, 256, 524, GB_IMMEDIATE, buttonMenuIndex, button_none, 22, 0},
	{0, 528, 256, 548, GB_IMMEDIATE, buttonMenuIndex, button_none, 23, 0},
	{0, 552, 256, 572, GB_IMMEDIATE, buttonMenuIndex, button_none, 24, 0},
	{0, 576, 256, 596, GB_IMMEDIATE, buttonMenuIndex, button_none, 25, 0},
	{0, 600, 256, 620, GB_IMMEDIATE, buttonMenuIndex, button_none, 26, 0},
	{0, 624, 256, 644, GB_IMMEDIATE, buttonMenuIndex, button_none, 27, 0},
	{0, 648, 256, 668, GB_IMMEDIATE, buttonMenuIndex, button_none, 28, 0},
	{0, 672, 256, 692, GB_IMMEDIATE, buttonMenuIndex, button_none, 29, 0},
	{0, 696, 256, 716, GB_IMMEDIATE, buttonMenuIndex, button_none, 30, 0},
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
	menu.numItems = building_menu_count_items(submenu);
	menu.yOffset = yMenuOffsets[menu.numItems];
	if (submenu == 0 || submenu == 1 || submenu == 2) {
		buttonMenuItem(0);
	} else {
		UI_Window_goTo(Window_BuildingMenu);
	}
}

void UI_BuildingMenu_drawSidebarImage(int xOffset, int forceDraw)
{
	if (city_view_is_sidebar_collapsed() && !forceDraw) {
		return;
	}
	if (building_construction_type() == BUILDING_NONE) {
		image_draw(image_group(GROUP_PANEL_WINDOWS) + 12, xOffset, 239);
		return;
	}
	int graphicPanelWindows = image_group(GROUP_PANEL_WINDOWS);
	int graphicId;
	switch (menu.selectedSubmenu) {
		default:
		case 0:
			graphicId = graphicPanelWindows;
			break;
		case 1:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(GROUP_PANEL_WINDOWS_DESERT);
			} else {
				graphicId = graphicPanelWindows + 11;
			}
			break;
		case 2:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(GROUP_PANEL_WINDOWS_DESERT) + 1;
			} else {
				graphicId = graphicPanelWindows + 10;
			}
			break;
		case 3:
			if (scenario_property_climate() == CLIMATE_DESERT) {
				graphicId = image_group(GROUP_PANEL_WINDOWS_DESERT) + 2;
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
				graphicId = image_group(GROUP_PANEL_WINDOWS_DESERT) + 3;
			} else {
				graphicId = graphicPanelWindows + 8;
			}
			break;
		case 11:
			graphicId = graphicPanelWindows + 9;
			break;
	}
	image_draw(graphicId, xOffset, 239);
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
		itemIndex = building_menu_next_index(menu.selectedSubmenu, itemIndex);
		label_draw(xOffset - 266, menu.yOffset + 110 + 24 * i,
			16, buildMenuFocusButtonId == i + 1 ? 1 : 2);
		int buildingType = building_menu_type(menu.selectedSubmenu, itemIndex);
		lang_text_draw_centered(28, buildingType,
			xOffset - 266, menu.yOffset + 113 + 24 * i, 176, FONT_NORMAL_GREEN);
		if (buildingType == BUILDING_DRAGGABLE_RESERVOIR) {
			buildingType = BUILDING_RESERVOIR;
		}
		int cost = model_get_building(buildingType)->cost;
		if (buildingType == BUILDING_FORT) {
			cost = 0;
		}
		if (cost) {
			text_draw_money(cost,
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

	if (!handleBuildSubmenu(m)) {
		UI_Sidebar_handleMouseBuildButtons(m);
	}
}

static int handleBuildSubmenu(const mouse *m)
{
	return generic_buttons_handle_mouse(
		m, Data_CityView.widthInPixels - 258, menu.yOffset + 110,
		buildMenuCustomButtons, menu.numItems, &buildMenuFocusButtonId);
}

static int buttonIndexToSubmenuItem(int index)
{
	int item = -1;
	for (int i = 0; i <= index; i++) {
		item = building_menu_next_index(menu.selectedSubmenu, item);
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

	Data_State.map.current.gridOffset = 0;

	building_type type = building_menu_type(menu.selectedSubmenu, item);
	building_construction_reset(type);

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
            default:
                break;
		}
		menu.numItems = building_menu_count_items(menu.selectedSubmenu);
		menu.yOffset = yMenuOffsets[menu.numItems];
		building_construction_clear_type();
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
            default:
                break;
		}
		UI_Window_goTo(Window_City);
	}
}
