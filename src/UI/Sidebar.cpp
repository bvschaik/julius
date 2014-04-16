#include "Sidebar.h"
#include "AllWindows.h"
#include "Minimap.h"
#include "Window.h"
#include "MessageDialog.h"
#include "../Graphics.h"
#include "../SidebarMenu.h"
#include "../Widget.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/State.h"

#define SIDEBAR_BORDER ((Data_Screen.width + 20) % 60)
#define XOFFSET_EXPANDED (Data_Screen.width - (Data_Screen.width + 20) % 60 - 162)

static void drawFillerBorders();
static void drawSidebar();
static void drawButtons();
static void drawOverlayText();
static void drawMinimap(int force);

static void buttonOverlay(int param1, int param2);
static void buttonCollapseExpand(int param1, int param2);
static void buttonBuild(int param1, int param2);
static void buttonUndo(int param1, int param2);
static void buttonMessages(int param1, int param2);
static void buttonHelp(int param1, int param2);
static void buttonGoToProblem(int param1, int param2);
static void buttonAdvisors(int param1, int param2);
static void buttonEmpire(int param1, int param2);
static void buttonMissionBriefing(int param1, int param2);
static void buttonRotateNorth(int param1, int param2);
static void buttonRotate(int param1, int param2);

static ImageButton buttonOverlaysCollapseSidebar[] = {
	{127, 5, 31, 20, 4, 90, 0, buttonCollapseExpand, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{4, 3, 117, 31, 4, 93, 0, buttonOverlay, buttonHelp, 1, 0, 0, 0, 0, 18}
};

static ImageButton buttonExpandSidebar[] = {
	{6, 4, 31, 20, 4, 90, 4, buttonCollapseExpand, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0}
};

static ImageButton buttonBuildCollapsed[] = {
	{2, 32, 39, 26, 2, 92, 0, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0},
	{2, 67, 39, 26, 2, 92, 8, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 2, 0},
	{2, 102, 39, 26, 2, 92, 12, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 3, 0},
	{2, 137, 39, 26, 2, 92, 4, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 4, 0},
	{2, 172, 39, 26, 2, 92, 40, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 5, 0},
	{2, 207, 39, 26, 2, 92, 28, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 6, 0},
	{2, 242, 39, 26, 2, 92, 24, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 7, 0},
	{2, 277, 39, 26, 2, 92, 20, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 8, 0},
	{2, 312, 39, 26, 2, 92, 16, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 9, 0},
	{2, 347, 39, 26, 2, 92, 44, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 10, 0},
	{2, 382, 39, 26, 2, 92, 36, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 11, 0},
	{2, 417, 39, 26, 2, 92, 32, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 12, 0},
};

static ImageButton buttonBuildExpanded[] = {
	{13, 277, 39, 26, 2, 92, 0, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0},
	{63, 277, 39, 26, 2, 92, 8, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 2, 0},
	{113, 277, 39, 26, 2, 92, 12, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 3, 0},
	{13, 313, 39, 26, 2, 92, 4, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 4, 0},
	{63, 313, 39, 26, 2, 92, 40, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 5, 0},
	{113, 313, 39, 26, 2, 92, 28, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 6, 0},
	{13, 349, 39, 26, 2, 92, 24, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 7, 0},
	{63, 349, 39, 26, 2, 92, 20, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 8, 0},
	{113, 349, 39, 26, 2, 92, 16, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 9, 0},
	{13, 385, 39, 26, 2, 92, 44, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 10, 0},
	{63, 385, 39, 26, 2, 92, 36, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 11, 0},
	{113, 385, 39, 26, 2, 92, 32, buttonBuild, Widget_Button_doNothing, 1, 0, 0, 0, 12, 0},
	{13, 421, 39, 26, 4, 92, 36, buttonUndo, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{63, 421, 39, 26, 4, 90, 18, buttonMessages, buttonHelp, 1, 0, 0, 0, 0, 34},
	{113, 421, 39, 26, 2, 90, 22, buttonGoToProblem, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
};

static ImageButton buttonTopExpanded[] = {
	{7, 155, 71, 23, 4, 13, 0, buttonAdvisors, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{84, 155, 71, 23, 4, 13, 3, buttonEmpire, buttonHelp, 1, 0, 0, 0, 0, 32},
	{7, 184, 33, 22, 4, 89, 0, buttonMissionBriefing, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{46, 184, 33, 22, 4, 89, 3, buttonRotateNorth, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{84, 184, 33, 22, 4, 89, 6, buttonRotate, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0},
	{123, 184, 33, 22, 4, 89, 9, buttonRotate, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0},
};

void UI_Sidebar_enableBuildingButtons()
{
	for (int i = 0; i < 12; i++) {
		buttonBuildExpanded[i].enabled = 1;
		if (SidebarMenu_countBuildingMenuItems(buttonBuildExpanded[i].parameter1)) {
			buttonBuildExpanded[i].enabled = 0;
		}

		buttonBuildCollapsed[i].enabled = 1;
		if (SidebarMenu_countBuildingMenuItems(buttonBuildCollapsed[i].parameter1)) {
			buttonBuildCollapsed[i].enabled = 0;
		}
	}
}

void UI_Sidebar_drawBackground()
{
	Data_State.sidebarCollapsed = 0;
	drawSidebar();
	drawFillerBorders();
	drawButtons();
}

static void drawSidebar()
{
	int graphicBase = GraphicId(ID_Graphic_SidePanel);
	int xOffsetPanel = Data_Screen.width - SIDEBAR_BORDER;
	if (Data_State.sidebarCollapsed) {
		xOffsetPanel -= 42;
		Graphics_drawImage(graphicBase, xOffsetPanel, 24);
	} else {
		xOffsetPanel -= 162;
		Graphics_drawImage(graphicBase + 1, xOffsetPanel, 24);
	}
	drawButtons();
	drawOverlayText();
    //j_fun_drawCitySidepanelBuildingGraphic(0, cityscreen_width_withControlpanel + 6);
    drawMinimap(1);
	//j_fun_drawCitySidepanelNumMessages(1);

	// relief images below panel
	int yOffset = 474;
	while (Data_Screen.width - yOffset > 0) {
		if (Data_Screen.width - yOffset <= 120) {
			Graphics_drawImage(graphicBase + 2 + Data_State.sidebarCollapsed, xOffsetPanel, yOffset);
			yOffset += 120;
		} else {
			Graphics_drawImage(graphicBase + 4 + Data_State.sidebarCollapsed, xOffsetPanel, yOffset);
			yOffset += 285;
		}
	}
}

static void drawFillerBorders()
{
	int borderRightWidth = SIDEBAR_BORDER;
	if (borderRightWidth) {
		int graphicId = GraphicId(ID_Graphic_TopMenuSidebar) + 13;
		if (borderRightWidth > 24) {
			// larger border
			graphicId -= 1;
		}
		int xOffset = Data_Screen.width - borderRightWidth;
		for (int yOffset = 24; yOffset < Data_Screen.height; yOffset += 24) {
			Graphics_drawImage(graphicId, xOffset, yOffset);
		}
	}

	int borderBottomHeight = (Data_Screen.height - 24) % 15;
	Graphics_fillRect(0, Data_Screen.height - borderBottomHeight, Data_Screen.width, borderBottomHeight, Color_Orange);
}

static void drawButtons()
{
	buttonBuildExpanded[12].enabled = Data_State.undoReady && Data_State.undoAvailable;
	if (Data_State.sidebarCollapsed) {
		int xOffset = Data_Screen.width - SIDEBAR_BORDER - 42;
		Widget_Button_drawImageButtons(xOffset, 24, buttonExpandSidebar, 1);
		Widget_Button_drawImageButtons(xOffset, 24, buttonBuildCollapsed, 12);
	} else {
		int xOffset = XOFFSET_EXPANDED;
		Widget_Button_drawImageButtons(xOffset, 24, buttonOverlaysCollapseSidebar, 2);
		Widget_Button_drawImageButtons(xOffset, 24, buttonBuildExpanded, 15);
		Widget_Button_drawImageButtons(xOffset, 24, buttonTopExpanded, 6);
	}
}

static void drawOverlayText()
{
	if (!Data_State.sidebarCollapsed) {
		int xOffset = XOFFSET_EXPANDED + 4;
		if (Data_State.currentOverlay) {
			Widget_GameText_drawCentered(14, Data_State.currentOverlay, xOffset, 32, 117, Font_NormalGreen);
		} else {
			Widget_GameText_drawCentered(6, 4, xOffset, 32, 117, Font_NormalGreen);
		}
	}
}

static void drawMinimap(int force)
{
	if (!Data_State.sidebarCollapsed) {
		int xOffset = XOFFSET_EXPANDED;
		// TODO force?
		UI_Minimap_draw(xOffset + 8, 59, 73, 111);
		Graphics_drawLine(xOffset + 7, 58, xOffset + 153, 58, Color_Minimap_Dark);
		Graphics_drawLine(xOffset + 7, 59, xOffset + 7, 170, Color_Minimap_Dark);
		Graphics_drawLine(xOffset + 153, 59, xOffset + 153, 170, Color_Minimap_Light);
	}
}

void UI_Sidebar_handleMouse()
{
	if (Data_State.sidebarCollapsed) {
		int xOffset = Data_Screen.width - SIDEBAR_BORDER - 42;
		if (!Widget_Button_handleImageButtons(xOffset, 24, buttonExpandSidebar, 1)) {
			Widget_Button_handleImageButtons(xOffset, 24, buttonBuildCollapsed, 12);
		}
	} else {
		int xOffset = XOFFSET_EXPANDED;
		if (!Widget_Button_handleImageButtons(xOffset, 24, buttonOverlaysCollapseSidebar, 2)) {
			if (!Widget_Button_handleImageButtons(xOffset, 24, buttonBuildExpanded, 15)) {
				Widget_Button_handleImageButtons(xOffset, 24, buttonTopExpanded, 6);
			}
		}
	}
}

void UI_Sidebar_handleMouseBuildButtons()
{
	if (Data_State.sidebarCollapsed) {
		int xOffset = Data_Screen.width - SIDEBAR_BORDER - 42;
		Widget_Button_handleImageButtonsClickOnly(xOffset, 24, buttonBuildCollapsed, 12);
	} else {
		int xOffset = XOFFSET_EXPANDED;
		Widget_Button_handleImageButtonsClickOnly(xOffset, 24, buttonBuildExpanded, 15);
	}
}

static void buttonOverlay(int param1, int param2)
{
	UI_Window_goTo(Window_OverlayMenu);
}

static void buttonCollapseExpand(int param1, int param2)
{
	// TODO
}

static void buttonBuild(int submenu, int param2)
{
	UI_BuildingMenu_init(submenu - 1);
}

static void buttonUndo(int param1, int param2)
{
	// TODO
}
static void buttonMessages(int param1, int param2)
{
	UI_Window_goTo(Window_PlayerMessageList);
}

static void buttonHelp(int param1, int param2)
{
	UI_MessageDialog_show(param2, 0);
}

static void buttonGoToProblem(int param1, int param2)
{
	// TODO
}
static void buttonAdvisors(int param1, int param2)
{
	// TODO
	UI_Window_goTo(Window_Advisors);
}
static void buttonEmpire(int param1, int param2)
{
	// TODO
	UI_Window_goTo(Window_Empire);
}
static void buttonMissionBriefing(int param1, int param2)
{
	// TODO
}
static void buttonRotateNorth(int param1, int param2)
{
	// TODO
}
static void buttonRotate(int param1, int param2)
{
	// TODO
}
