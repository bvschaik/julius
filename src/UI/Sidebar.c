#include "Sidebar.h"

#include "AllWindows.h"
#include "Advisors.h"
#include "MessageDialog.h"
#include "Minimap.h"

#include "../CityView.h"
#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/Screen.h"
#include "../Data/State.h"

#include "building/menu.h"
#include "city/message.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/direction.h"
#include "game/state.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "input/scroll.h"
#include "map/orientation.h"
#include "scenario/property.h"
#include "sound/effect.h"

#define SIDEBAR_BORDER ((Data_Screen.width + 20) % 60)
#define BOTTOM_BORDER ((Data_Screen.height - 24) % 15)
#define XOFFSET_EXPANDED (Data_Screen.width - (Data_Screen.width + 20) % 60 - 162)

static void drawNumberOfMessages();
static void drawFillerBorders();
static void drawSidebar();
static void drawButtons();
static void drawOverlayText(int xOffset);

static void buttonOverlay(int param1, int param2);
static void buttonCollapseExpand(int param1, int param2);
static void buttonBuild(int submenu, int param2);
static void buttonUndo(int param1, int param2);
static void buttonMessages(int param1, int param2);
static void buttonHelp(int param1, int param2);
static void buttonGoToProblem(int param1, int param2);
static void buttonAdvisors(int param1, int param2);
static void buttonEmpire(int param1, int param2);
static void buttonMissionBriefing(int param1, int param2);
static void buttonRotateNorth(int param1, int param2);
static void buttonRotate(int clockWise, int param2);

static ImageButton buttonOverlaysCollapseSidebar[] = {
	{127, 5, 31, 20, ImageButton_Normal, 90, 0, buttonCollapseExpand, Widget_Button_doNothing, 0, 0, 1},
	{4, 3, 117, 31, ImageButton_Normal, 93, 0, buttonOverlay, buttonHelp, 0, 0, 1}
};

static ImageButton buttonExpandSidebar[] = {
	{6, 4, 31, 20, ImageButton_Normal, 90, 4, buttonCollapseExpand, Widget_Button_doNothing, 0, 0, 1}
};

static ImageButton buttonBuildCollapsed[] = {
	{2, 32, 39, 26, ImageButton_Normal, 92, 0, buttonBuild, Widget_Button_doNothing, 0, 0, 1},
	{2, 67, 39, 26, ImageButton_Normal, 92, 8, buttonBuild, Widget_Button_doNothing, 1, 0, 1},
	{2, 102, 39, 26, ImageButton_Normal, 92, 12, buttonBuild, Widget_Button_doNothing, 2, 0, 1},
	{2, 137, 39, 26, ImageButton_Build, 92, 4, buttonBuild, Widget_Button_doNothing, 3, 0, 1},
	{2, 172, 39, 26, ImageButton_Build, 92, 40, buttonBuild, Widget_Button_doNothing, 4, 0, 1},
	{2, 207, 39, 26, ImageButton_Build, 92, 28, buttonBuild, Widget_Button_doNothing, 5, 0, 1},
	{2, 242, 39, 26, ImageButton_Build, 92, 24, buttonBuild, Widget_Button_doNothing, 6, 0, 1},
	{2, 277, 39, 26, ImageButton_Build, 92, 20, buttonBuild, Widget_Button_doNothing, 7, 0, 1},
	{2, 312, 39, 26, ImageButton_Build, 92, 16, buttonBuild, Widget_Button_doNothing, 8, 0, 1},
	{2, 347, 39, 26, ImageButton_Build, 92, 44, buttonBuild, Widget_Button_doNothing, 9, 0, 1},
	{2, 382, 39, 26, ImageButton_Build, 92, 36, buttonBuild, Widget_Button_doNothing, 10, 0, 1},
	{2, 417, 39, 26, ImageButton_Build, 92, 32, buttonBuild, Widget_Button_doNothing, 11, 0, 1},
};

static ImageButton buttonBuildExpanded[] = {
	{13, 277, 39, 26, ImageButton_Normal, 92, 0, buttonBuild, Widget_Button_doNothing, 0, 0, 1},
	{63, 277, 39, 26, ImageButton_Normal, 92, 8, buttonBuild, Widget_Button_doNothing, 1, 0, 1},
	{113, 277, 39, 26, ImageButton_Normal, 92, 12, buttonBuild, Widget_Button_doNothing, 2, 0, 1},
	{13, 313, 39, 26, ImageButton_Build, 92, 4, buttonBuild, Widget_Button_doNothing, 3, 0, 1},
	{63, 313, 39, 26, ImageButton_Build, 92, 40, buttonBuild, Widget_Button_doNothing, 4, 0, 1},
	{113, 313, 39, 26, ImageButton_Build, 92, 28, buttonBuild, Widget_Button_doNothing, 5, 0, 1},
	{13, 349, 39, 26, ImageButton_Build, 92, 24, buttonBuild, Widget_Button_doNothing, 6, 0, 1},
	{63, 349, 39, 26, ImageButton_Build, 92, 20, buttonBuild, Widget_Button_doNothing, 7, 0, 1},
	{113, 349, 39, 26, ImageButton_Build, 92, 16, buttonBuild, Widget_Button_doNothing, 8, 0, 1},
	{13, 385, 39, 26, ImageButton_Build, 92, 44, buttonBuild, Widget_Button_doNothing, 9, 0, 1},
	{63, 385, 39, 26, ImageButton_Build, 92, 36, buttonBuild, Widget_Button_doNothing, 10, 0, 1},
	{113, 385, 39, 26, ImageButton_Build, 92, 32, buttonBuild, Widget_Button_doNothing, 11, 0, 1},
	{13, 421, 39, 26, ImageButton_Normal, 92, 48, buttonUndo, Widget_Button_doNothing, 0, 0, 1},
	{63, 421, 39, 26, ImageButton_Normal, 90, 18, buttonMessages, buttonHelp, 0, 0, 1},
	{113, 421, 39, 26, ImageButton_Build, 90, 22, buttonGoToProblem, Widget_Button_doNothing, 0, 0, 1},
};

static ImageButton buttonTopExpanded[] = {
	{7, 155, 71, 23, ImageButton_Normal, 13, 0, buttonAdvisors, Widget_Button_doNothing, 0, 0, 1},
	{84, 155, 71, 23, ImageButton_Normal, 13, 3, buttonEmpire, buttonHelp, 0, 0, 1},
	{7, 184, 33, 22, ImageButton_Normal, 89, 0, buttonMissionBriefing, Widget_Button_doNothing, 0, 0, 1},
	{46, 184, 33, 22, ImageButton_Normal, 89, 3, buttonRotateNorth, Widget_Button_doNothing, 0, 0, 1},
	{84, 184, 33, 22, ImageButton_Normal, 89, 6, buttonRotate, Widget_Button_doNothing, 0, 0, 1},
	{123, 184, 33, 22, ImageButton_Normal, 89, 9, buttonRotate, Widget_Button_doNothing, 1, 0, 1},
};

static int minimapRedrawRequested = 0;

// sliding sidebar stuff
static const int progressToOffset[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 21, 24, 27,
	30, 33, 37, 41, 45, 49, 54, 59, 64, 70, 76, 83, 91, 99, 106, 113,
	119, 125, 130, 135, 139, 143, 146, 149, 152, 154, 156, 158, 160, 162, 165
};

static struct {
	time_millis slideStart;
	int progress;
	int focusButtonForTooltip;
} data;

void UI_Sidebar_requestMinimapRefresh()
{
	minimapRedrawRequested = 1;
}

static void enableBuildingButtons()
{
	for (int i = 0; i < 12; i++) {
		buttonBuildExpanded[i].enabled = 1;
		if (building_menu_count_items(buttonBuildExpanded[i].parameter1) <= 0) {
			buttonBuildExpanded[i].enabled = 0;
		}

		buttonBuildCollapsed[i].enabled = 1;
		if (building_menu_count_items(buttonBuildCollapsed[i].parameter1) <= 0) {
			buttonBuildCollapsed[i].enabled = 0;
		}
	}
}

void UI_Sidebar_drawBackground()
{
	drawSidebar();
	drawFillerBorders();
}

void UI_Sidebar_drawForeground()
{
    if (building_menu_has_changed()) {
        enableBuildingButtons();
    }
	int xOffsetPanel = Data_Screen.width - SIDEBAR_BORDER;
	if (Data_State.sidebarCollapsed) {
		xOffsetPanel -= 42;
	} else {
		xOffsetPanel -= 162;
	}
	drawButtons();
	drawOverlayText(xOffsetPanel + 4);
	UI_Sidebar_drawMinimap(0);
	drawNumberOfMessages();
}

static void drawNumberOfMessages()
{
	if (UI_Window_getId() == Window_City && !Data_State.sidebarCollapsed) {
        int messages = city_message_count();
		buttonBuildExpanded[12].enabled = game_can_undo();
		buttonBuildExpanded[13].enabled = messages > 0;
		buttonBuildExpanded[14].enabled = city_message_problem_area_count();
		if (messages) {
			Widget_Text_drawNumberCenteredColored(messages,
				XOFFSET_EXPANDED + 74, 452, 32, FONT_SMALL_PLAIN, COLOR_BLACK);
			Widget_Text_drawNumberCenteredColored(messages,
				XOFFSET_EXPANDED + 73, 453, 32, FONT_SMALL_PLAIN, COLOR_WHITE);
		}
	}
}

static void drawSidebar()
{
	int graphicBase = image_group(GROUP_SIDE_PANEL);
	int xOffsetPanel = Data_Screen.width - SIDEBAR_BORDER;
	if (Data_State.sidebarCollapsed) {
		xOffsetPanel -= 42;
		Graphics_drawImage(graphicBase, xOffsetPanel, 24);
	} else {
		xOffsetPanel -= 162;
		Graphics_drawImage(graphicBase + 1, xOffsetPanel, 24);
	}
	drawButtons();
	drawOverlayText(xOffsetPanel + 4);
	UI_BuildingMenu_drawSidebarImage(xOffsetPanel + 6, 0);
	UI_Sidebar_drawMinimap(1);

	// relief images below panel
	int yOffset = 474;
	int yMax = Data_Screen.height - BOTTOM_BORDER;
	while (yOffset < yMax) {
		if (yMax - yOffset <= 120) {
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
		int graphicId = image_group(GROUP_TOP_MENU_SIDEBAR) + 13;
		if (borderRightWidth > 24) {
			// larger border
			graphicId -= 1;
		}
		int xOffset = Data_Screen.width - borderRightWidth;
		for (int yOffset = 24; yOffset < Data_Screen.height; yOffset += 24) {
			Graphics_drawImage(graphicId, xOffset, yOffset);
		}
	}

	int borderBottomHeight = BOTTOM_BORDER;
	Graphics_fillRect(0, Data_Screen.height - borderBottomHeight, Data_Screen.width, borderBottomHeight, COLOR_BLACK);
}

static void drawButtons()
{
	buttonBuildExpanded[12].enabled = game_can_undo();
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

static void drawOverlayText(int xOffset)
{
	if (!Data_State.sidebarCollapsed) {
		if (game_state_overlay()) {
			Widget_GameText_drawCentered(14, game_state_overlay(), xOffset, 32, 117, FONT_NORMAL_GREEN);
		} else {
			Widget_GameText_drawCentered(6, 4, xOffset, 32, 117, FONT_NORMAL_GREEN);
		}
	}
}

void UI_Sidebar_drawMinimap(int force)
{
	if (!Data_State.sidebarCollapsed) {
		if (minimapRedrawRequested || scroll_in_progress() || force) {
			int xOffset = XOFFSET_EXPANDED;
			UI_Minimap_draw(xOffset + 8, 59, 73, 111);
			Graphics_drawLine(xOffset + 7, 58, xOffset + 153, 58, COLOR_MINIMAP_DARK);
			Graphics_drawLine(xOffset + 7, 59, xOffset + 7, 170, COLOR_MINIMAP_DARK);
			Graphics_drawLine(xOffset + 153, 59, xOffset + 153, 170, COLOR_MINIMAP_LIGHT);
		}
	}
}

int UI_Sidebar_handleMouse(const mouse *m)
{
	int buttonId;
	data.focusButtonForTooltip = 0;
	if (Data_State.sidebarCollapsed) {
		int xOffset = Data_Screen.width - SIDEBAR_BORDER - 42;
		Widget_Button_handleImageButtons(xOffset, 24, buttonExpandSidebar, 1, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = 12;
		}
		Widget_Button_handleImageButtons(xOffset, 24, buttonBuildCollapsed, 12, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 19;
		}
	} else {
		if (UI_Minimap_handleClick(m)) {
			return 1;
		}
		int xOffset = XOFFSET_EXPANDED;
		Widget_Button_handleImageButtons(xOffset, 24, buttonOverlaysCollapseSidebar, 2, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 9;
		}
		Widget_Button_handleImageButtons(xOffset, 24, buttonBuildExpanded, 15, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 19;
		}
		Widget_Button_handleImageButtons(xOffset, 24, buttonTopExpanded, 6, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 39;
		}
	}
	return buttonId != 0;
}

void UI_Sidebar_handleMouseBuildButtons(const mouse *m)
{
	if (Data_State.sidebarCollapsed) {
		int xOffset = Data_Screen.width - SIDEBAR_BORDER - 42;
		Widget_Button_handleImageButtons(xOffset, 24, buttonBuildCollapsed, 12, 0);
	} else {
		int xOffset = XOFFSET_EXPANDED;
		Widget_Button_handleImageButtons(xOffset, 24, buttonBuildExpanded, 15, 0);
	}
}

int UI_Sidebar_getTooltipText()
{
	if (Data_State.sidebarCollapsed && Data_State.sidebarCollapsed != 1) {
		return 0;
	}
	return data.focusButtonForTooltip;
}

static void buttonOverlay(int param1, int param2)
{
	UI_Window_goTo(Window_OverlayMenu);
}

static void buttonCollapseExpand(int param1, int param2)
{
	data.progress = 0;
	data.slideStart = time_get_millis();
	UI_Window_goTo(Window_SlidingSidebar);
	CityView_setViewportWithoutSidebar();
	city_view_check_camera_boundaries();
	sound_effect_play(SOUND_EFFECT_SIDEBAR);
}

static void buttonBuild(int submenu, int param2)
{
	UI_BuildingMenu_init(submenu);
}

static void buttonUndo(int param1, int param2)
{
	game_undo_perform();
	UI_Window_requestRefresh();
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
    int gridOffset = city_message_next_problem_area_grid_offset();
    if (gridOffset) {
        city_view_go_to_grid_offset(gridOffset);
        UI_Window_goTo(Window_City);
    } else {
        UI_Window_requestRefresh();
    }
}

static void buttonAdvisors(int param1, int param2)
{
	UI_Advisors_goToFromSidepanel();
}

static void buttonEmpire(int param1, int param2)
{
    switch (tutorial_advisor_empire_availability()) {
        case NOT_AVAILABLE:
            city_warning_show(WARNING_NOT_AVAILABLE);
            break;
        case NOT_AVAILABLE_YET:
            city_warning_show(WARNING_NOT_AVAILABLE_YET);
            break;
        case AVAILABLE:
            UI_Window_goTo(Window_Empire);
            break;
    }
}
static void buttonMissionBriefing(int param1, int param2)
{
	if (!scenario_is_custom()) {
		UI_Intermezzo_show(Intermezzo_MissionBriefing, Window_MissionBriefingReview, 1000);
	}
}

static void buttonRotateNorth(int param1, int param2)
{
	switch (Data_State.map.orientation) {
		case DIR_0_TOP: // already north
			return;
		case DIR_2_RIGHT:
			city_view_rotate_right();
			map_orientation_change(1);
			break;
		case DIR_4_BOTTOM:
			city_view_rotate_left();
			map_orientation_change(0);
			// fallthrough
		case DIR_6_LEFT:
			city_view_rotate_left();
			map_orientation_change(0);
			break;
	}
	city_view_check_camera_boundaries();
	city_warning_show(WARNING_ORIENTATION);
	UI_Window_requestRefresh();
}

static void buttonRotate(int clockWise, int param2)
{
	if (clockWise) {
		city_view_rotate_right();
	} else {
		city_view_rotate_left();
	}
	map_orientation_change(clockWise);
	city_view_check_camera_boundaries();
	city_warning_show(WARNING_ORIENTATION);
	UI_Window_requestRefresh();
}

void UI_Sidebar_rotateMap(int clockWise)
{
	buttonRotate(clockWise, 0);
}

static void updateProgress()
{
	time_millis now = time_get_millis();
	time_millis diff = now - data.slideStart;
	data.progress = diff / 10;
}

void UI_SlidingSidebar_drawBackground()
{
	UI_City_drawCity();
}

void UI_SlidingSidebar_drawForeground()
{
	UI_Window_requestRefresh();
	updateProgress();
	if (data.progress >= 47) {
		if (Data_State.sidebarCollapsed) {
			Data_State.sidebarCollapsed = 0;
			CityView_setViewportWithSidebar();
		} else {
			Data_State.sidebarCollapsed = 1;
			CityView_setViewportWithoutSidebar();
		}
		city_view_check_camera_boundaries();
		UI_Window_goTo(Window_City);
		UI_Window_refresh(1);
		return;
	}

	Graphics_setClipRectangle(
		Data_Screen.width - SIDEBAR_BORDER - 162, 24,
		162, Data_Screen.height - 24 - BOTTOM_BORDER);

	int graphicBase = image_group(GROUP_SIDE_PANEL);
	// draw collapsed sidebar
	int xOffsetCollapsed = Data_Screen.width - SIDEBAR_BORDER - 42;
	Graphics_drawImage(graphicBase, xOffsetCollapsed, 24);
	Widget_Button_drawImageButtons(xOffsetCollapsed, 24, buttonExpandSidebar, 1);
	Widget_Button_drawImageButtons(xOffsetCollapsed, 24, buttonBuildCollapsed, 12);

	// draw expanded sidebar on top of it
	int xOffsetExpanded = XOFFSET_EXPANDED;
	if (Data_State.sidebarCollapsed) {
		xOffsetExpanded += progressToOffset[47 - data.progress];
	} else {
		xOffsetExpanded += progressToOffset[data.progress];
	}
	Graphics_drawImage(graphicBase + 1, xOffsetExpanded, 24);
	Widget_Button_drawImageButtons(xOffsetExpanded, 24, buttonOverlaysCollapseSidebar, 2);
	Widget_Button_drawImageButtons(xOffsetExpanded, 24, buttonBuildExpanded, 15);
	Widget_Button_drawImageButtons(xOffsetExpanded, 24, buttonTopExpanded, 6);

	// black out minimap
	Graphics_fillRect(xOffsetExpanded + 8, 59, 145, 111, COLOR_BLACK);

	drawOverlayText(xOffsetExpanded + 4);
	UI_BuildingMenu_drawSidebarImage(xOffsetExpanded + 6, 1);

	// relief images below buttons
	int yOffset = 474;
	while (Data_Screen.width - yOffset > 0) {
		if (Data_Screen.width - yOffset <= 120) {
			Graphics_drawImage(graphicBase + 3, xOffsetCollapsed, yOffset);
			Graphics_drawImage(graphicBase + 2, xOffsetExpanded, yOffset);
			yOffset += 120;
		} else {
			Graphics_drawImage(graphicBase + 5, xOffsetCollapsed, yOffset);
			Graphics_drawImage(graphicBase + 4, xOffsetExpanded, yOffset);
			yOffset += 285;
		}
	}
	Graphics_resetClipRectangle();
}

