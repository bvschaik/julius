#include "Sidebar.h"

#include "Minimap.h"

#include "building/menu.h"
#include "city/message.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/direction.h"
#include "game/orientation.h"
#include "game/state.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "map/orientation.h"
#include "scenario/property.h"
#include "sound/effect.h"
#include "window/advisors.h"
#include "window/build_menu.h"
#include "window/city.h"
#include "window/empire.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_briefing.h"
#include "window/overlay_menu.h"

#define SIDEBAR_BORDER ((screen_width() + 20) % 60)
#define BOTTOM_BORDER ((screen_height() - 24) % 15)
#define XOFFSET_EXPANDED (screen_width() - (screen_width() + 20) % 60 - 162)

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

static image_button buttonOverlaysCollapseSidebar[] = {
	{127, 5, 31, 20, IB_NORMAL, 90, 0, buttonCollapseExpand, button_none, 0, 0, 1},
	{4, 3, 117, 31, IB_NORMAL, 93, 0, buttonOverlay, buttonHelp, 0, 0, 1}
};

static image_button buttonExpandSidebar[] = {
	{6, 4, 31, 20, IB_NORMAL, 90, 4, buttonCollapseExpand, button_none, 0, 0, 1}
};

static image_button buttonBuildCollapsed[] = {
	{2, 32, 39, 26, IB_NORMAL, 92, 0, buttonBuild, button_none, 0, 0, 1},
	{2, 67, 39, 26, IB_NORMAL, 92, 8, buttonBuild, button_none, 1, 0, 1},
	{2, 102, 39, 26, IB_NORMAL, 92, 12, buttonBuild, button_none, 2, 0, 1},
	{2, 137, 39, 26, IB_BUILD, 92, 4, buttonBuild, button_none, 3, 0, 1},
	{2, 172, 39, 26, IB_BUILD, 92, 40, buttonBuild, button_none, 4, 0, 1},
	{2, 207, 39, 26, IB_BUILD, 92, 28, buttonBuild, button_none, 5, 0, 1},
	{2, 242, 39, 26, IB_BUILD, 92, 24, buttonBuild, button_none, 6, 0, 1},
	{2, 277, 39, 26, IB_BUILD, 92, 20, buttonBuild, button_none, 7, 0, 1},
	{2, 312, 39, 26, IB_BUILD, 92, 16, buttonBuild, button_none, 8, 0, 1},
	{2, 347, 39, 26, IB_BUILD, 92, 44, buttonBuild, button_none, 9, 0, 1},
	{2, 382, 39, 26, IB_BUILD, 92, 36, buttonBuild, button_none, 10, 0, 1},
	{2, 417, 39, 26, IB_BUILD, 92, 32, buttonBuild, button_none, 11, 0, 1},
};

static image_button buttonBuildExpanded[] = {
	{13, 277, 39, 26, IB_NORMAL, 92, 0, buttonBuild, button_none, 0, 0, 1},
	{63, 277, 39, 26, IB_NORMAL, 92, 8, buttonBuild, button_none, 1, 0, 1},
	{113, 277, 39, 26, IB_NORMAL, 92, 12, buttonBuild, button_none, 2, 0, 1},
	{13, 313, 39, 26, IB_BUILD, 92, 4, buttonBuild, button_none, 3, 0, 1},
	{63, 313, 39, 26, IB_BUILD, 92, 40, buttonBuild, button_none, 4, 0, 1},
	{113, 313, 39, 26, IB_BUILD, 92, 28, buttonBuild, button_none, 5, 0, 1},
	{13, 349, 39, 26, IB_BUILD, 92, 24, buttonBuild, button_none, 6, 0, 1},
	{63, 349, 39, 26, IB_BUILD, 92, 20, buttonBuild, button_none, 7, 0, 1},
	{113, 349, 39, 26, IB_BUILD, 92, 16, buttonBuild, button_none, 8, 0, 1},
	{13, 385, 39, 26, IB_BUILD, 92, 44, buttonBuild, button_none, 9, 0, 1},
	{63, 385, 39, 26, IB_BUILD, 92, 36, buttonBuild, button_none, 10, 0, 1},
	{113, 385, 39, 26, IB_BUILD, 92, 32, buttonBuild, button_none, 11, 0, 1},
	{13, 421, 39, 26, IB_NORMAL, 92, 48, buttonUndo, button_none, 0, 0, 1},
	{63, 421, 39, 26, IB_NORMAL, 90, 18, buttonMessages, buttonHelp, 0, 0, 1},
	{113, 421, 39, 26, IB_BUILD, 90, 22, buttonGoToProblem, button_none, 0, 0, 1},
};

static image_button buttonTopExpanded[] = {
	{7, 155, 71, 23, IB_NORMAL, 13, 0, buttonAdvisors, button_none, 0, 0, 1},
	{84, 155, 71, 23, IB_NORMAL, 13, 3, buttonEmpire, buttonHelp, 0, 0, 1},
	{7, 184, 33, 22, IB_NORMAL, 89, 0, buttonMissionBriefing, button_none, 0, 0, 1},
	{46, 184, 33, 22, IB_NORMAL, 89, 3, buttonRotateNorth, button_none, 0, 0, 1},
	{84, 184, 33, 22, IB_NORMAL, 89, 6, buttonRotate, button_none, 0, 0, 1},
	{123, 184, 33, 22, IB_NORMAL, 89, 9, buttonRotate, button_none, 1, 0, 1},
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
	int xOffsetPanel = screen_width() - SIDEBAR_BORDER;
	if (city_view_is_sidebar_collapsed()) {
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
	if (window_is(Window_City) && !city_view_is_sidebar_collapsed()) {
        int messages = city_message_count();
		buttonBuildExpanded[12].enabled = game_can_undo();
		buttonBuildExpanded[13].enabled = messages > 0;
		buttonBuildExpanded[14].enabled = city_message_problem_area_count();
		if (messages) {
			text_draw_number_centered_colored(messages,
				XOFFSET_EXPANDED + 74, 452, 32, FONT_SMALL_PLAIN, COLOR_BLACK);
			text_draw_number_centered_colored(messages,
				XOFFSET_EXPANDED + 73, 453, 32, FONT_SMALL_PLAIN, COLOR_WHITE);
		}
	}
}

static void drawBuildImage(int x_offset, int force)
{
    if (city_view_is_sidebar_collapsed() && !force) {
        return;
    }
    image_draw(window_build_menu_image(), x_offset, 239);
}

static void drawSidebar()
{
	int graphicBase = image_group(GROUP_SIDE_PANEL);
	int xOffsetPanel = screen_width() - SIDEBAR_BORDER;
	if (city_view_is_sidebar_collapsed()) {
		xOffsetPanel -= 42;
		image_draw(graphicBase, xOffsetPanel, 24);
	} else {
		xOffsetPanel -= 162;
		image_draw(graphicBase + 1, xOffsetPanel, 24);
	}
	drawButtons();
	drawOverlayText(xOffsetPanel + 4);
	drawBuildImage(xOffsetPanel + 6, 0);
	UI_Sidebar_drawMinimap(1);

	// relief images below panel
	int yOffset = 474;
	int yMax = screen_height() - BOTTOM_BORDER;
	while (yOffset < yMax) {
		if (yMax - yOffset <= 120) {
			image_draw(graphicBase + 2 + city_view_is_sidebar_collapsed(), xOffsetPanel, yOffset);
			yOffset += 120;
		} else {
			image_draw(graphicBase + 4 + city_view_is_sidebar_collapsed(), xOffsetPanel, yOffset);
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
        if (borderRightWidth > 40) {
            int xOffset = screen_width() - 35;
            for (int yOffset = 24; yOffset < screen_height(); yOffset += 24) {
                image_draw(graphicId, xOffset, yOffset);
            }
        }
		int xOffset = screen_width() - borderRightWidth;
		for (int yOffset = 24; yOffset < screen_height(); yOffset += 24) {
			image_draw(graphicId, xOffset, yOffset);
		}
	}

	int borderBottomHeight = BOTTOM_BORDER;
	graphics_fill_rect(0, screen_height() - borderBottomHeight, screen_width(), borderBottomHeight, COLOR_BLACK);
}

static void drawButtons()
{
	buttonBuildExpanded[12].enabled = game_can_undo();
	if (city_view_is_sidebar_collapsed()) {
		int xOffset = screen_width() - SIDEBAR_BORDER - 42;
		image_buttons_draw(xOffset, 24, buttonExpandSidebar, 1);
		image_buttons_draw(xOffset, 24, buttonBuildCollapsed, 12);
	} else {
		int xOffset = XOFFSET_EXPANDED;
		image_buttons_draw(xOffset, 24, buttonOverlaysCollapseSidebar, 2);
		image_buttons_draw(xOffset, 24, buttonBuildExpanded, 15);
		image_buttons_draw(xOffset, 24, buttonTopExpanded, 6);
	}
}

static void drawOverlayText(int xOffset)
{
	if (!city_view_is_sidebar_collapsed()) {
		if (game_state_overlay()) {
			lang_text_draw_centered(14, game_state_overlay(), xOffset, 32, 117, FONT_NORMAL_GREEN);
		} else {
			lang_text_draw_centered(6, 4, xOffset, 32, 117, FONT_NORMAL_GREEN);
		}
	}
}

void UI_Sidebar_drawMinimap(int force)
{
	if (!city_view_is_sidebar_collapsed()) {
		if (minimapRedrawRequested || scroll_in_progress() || force) {
			int xOffset = XOFFSET_EXPANDED;
			UI_Minimap_draw(xOffset + 8, 59, 73, 111);
			graphics_draw_line(xOffset + 7, 58, xOffset + 153, 58, COLOR_MINIMAP_DARK);
			graphics_draw_line(xOffset + 7, 59, xOffset + 7, 170, COLOR_MINIMAP_DARK);
			graphics_draw_line(xOffset + 153, 59, xOffset + 153, 170, COLOR_MINIMAP_LIGHT);
		}
	}
}

int UI_Sidebar_handleMouse(const mouse *m)
{
	int buttonId;
	data.focusButtonForTooltip = 0;
	if (city_view_is_sidebar_collapsed()) {
        int x_offset = screen_width() - SIDEBAR_BORDER - 42;
		image_buttons_handle_mouse(m, x_offset, 24, buttonExpandSidebar, 1, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = 12;
		}
		image_buttons_handle_mouse(m, x_offset, 24, buttonBuildCollapsed, 12, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 19;
		}
	} else {
		if (UI_Minimap_handleClick(m)) {
			return 1;
		}
        int x_offset = XOFFSET_EXPANDED;
		image_buttons_handle_mouse(m, x_offset, 24, buttonOverlaysCollapseSidebar, 2, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 9;
		}
		image_buttons_handle_mouse(m, x_offset, 24, buttonBuildExpanded, 15, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 19;
		}
		image_buttons_handle_mouse(m, x_offset, 24, buttonTopExpanded, 6, &buttonId);
		if (buttonId) {
			data.focusButtonForTooltip = buttonId + 39;
		}
	}
	return buttonId != 0;
}

void UI_Sidebar_handleMouseBuildButtons(const mouse *m)
{
	if (city_view_is_sidebar_collapsed()) {
		image_buttons_handle_mouse(m, screen_width() - SIDEBAR_BORDER - 42, 24, buttonBuildCollapsed, 12, 0);
	} else {
		image_buttons_handle_mouse(m, XOFFSET_EXPANDED, 24, buttonBuildExpanded, 15, 0);
	}
}

int UI_Sidebar_getTooltipText()
{
	return data.focusButtonForTooltip;
}

static void buttonOverlay(int param1, int param2)
{
	window_overlay_menu_show();
}

static void buttonCollapseExpand(int param1, int param2)
{
	data.progress = 0;
	data.slideStart = time_get_millis();
	UI_Window_goTo(Window_SlidingSidebar);
	city_view_start_sidebar_toggle();
	sound_effect_play(SOUND_EFFECT_SIDEBAR);
}

static void buttonBuild(int submenu, int param2)
{
	window_build_menu_show(submenu);
}

static void buttonUndo(int param1, int param2)
{
	game_undo_perform();
	window_invalidate();
}

static void buttonMessages(int param1, int param2)
{
	window_message_list_show();
}

static void buttonHelp(int param1, int param2)
{
	window_message_dialog_show(param2, 0);
}

static void buttonGoToProblem(int param1, int param2)
{
    int gridOffset = city_message_next_problem_area_grid_offset();
    if (gridOffset) {
        city_view_go_to_grid_offset(gridOffset);
        window_city_show();
    } else {
        window_invalidate();
    }
}

static void buttonAdvisors(int param1, int param2)
{
	window_advisors_show_checked();
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
            window_empire_show();
            break;
    }
}
static void buttonMissionBriefing(int param1, int param2)
{
	if (!scenario_is_custom()) {
		window_mission_briefing_show_review();
	}
}

static void buttonRotateNorth(int param1, int param2)
{
	game_orientation_rotate_north();
	window_invalidate();
}

static void buttonRotate(int clockWise, int param2)
{
	if (clockWise) {
		game_orientation_rotate_right();
	} else {
		game_orientation_rotate_left();
	}
	window_invalidate();
}

static void updateProgress()
{
	time_millis now = time_get_millis();
	time_millis diff = now - data.slideStart;
	data.progress = diff / 10;
}

void UI_SlidingSidebar_drawBackground()
{
	window_city_draw();
}

void UI_SlidingSidebar_drawForeground()
{
	window_invalidate();
	updateProgress();
	if (data.progress >= 47) {
		city_view_toggle_sidebar();
		window_city_show();
		window_draw(1);
		return;
	}

	graphics_set_clip_rectangle(
		screen_width() - SIDEBAR_BORDER - 162, 24,
		162, screen_height() - 24 - BOTTOM_BORDER);

	int graphicBase = image_group(GROUP_SIDE_PANEL);
	// draw collapsed sidebar
	int xOffsetCollapsed = screen_width() - SIDEBAR_BORDER - 42;
	image_draw(graphicBase, xOffsetCollapsed, 24);
	image_buttons_draw(xOffsetCollapsed, 24, buttonExpandSidebar, 1);
	image_buttons_draw(xOffsetCollapsed, 24, buttonBuildCollapsed, 12);

	// draw expanded sidebar on top of it
	int xOffsetExpanded = XOFFSET_EXPANDED;
	if (city_view_is_sidebar_collapsed()) {
		xOffsetExpanded += progressToOffset[47 - data.progress];
	} else {
		xOffsetExpanded += progressToOffset[data.progress];
	}
	image_draw(graphicBase + 1, xOffsetExpanded, 24);
	image_buttons_draw(xOffsetExpanded, 24, buttonOverlaysCollapseSidebar, 2);
	image_buttons_draw(xOffsetExpanded, 24, buttonBuildExpanded, 15);
	image_buttons_draw(xOffsetExpanded, 24, buttonTopExpanded, 6);

	// black out minimap
	graphics_fill_rect(xOffsetExpanded + 8, 59, 145, 111, COLOR_BLACK);

	drawOverlayText(xOffsetExpanded + 4);
	drawBuildImage(xOffsetExpanded + 6, 1);

	// relief images below buttons
	int yOffset = 474;
	while (screen_width() - yOffset > 0) {
		if (screen_width() - yOffset <= 120) {
			image_draw(graphicBase + 3, xOffsetCollapsed, yOffset);
			image_draw(graphicBase + 2, xOffsetExpanded, yOffset);
			yOffset += 120;
		} else {
			image_draw(graphicBase + 5, xOffsetCollapsed, yOffset);
			image_draw(graphicBase + 4, xOffsetExpanded, yOffset);
			yOffset += 285;
		}
	}
	graphics_reset_clip_rectangle();
}

