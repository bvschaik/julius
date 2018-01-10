#include "window.h"

#include "input/cursor.h"

#include "UI/AllWindows.h"
#include "UI/BuildingInfo.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"
#include "UI/Sidebar.h"
#include "UI/TopMenu.h"
#include "UI/VideoIntermezzo.h"
#include "UI/Warning.h"

static void noop()
{
}

static window_type windows[] = {
    // 0
    { Window_MainMenu },
    { Window_City, UI_City_drawBackground, UI_City_drawForeground, UI_City_handleMouse, UI_City_getTooltip, noop },
    { Window_PopupDialog },
    { Window_TopMenu, UI_City_drawBackground, UI_TopMenu_drawForeground, UI_TopMenu_handleMouse, noop, noop },
    { Window_DifficultyOptions, noop, noop, noop, noop, noop },
    { Window_Advisors, UI_Advisors_drawBackground, UI_Advisors_drawForeground, UI_Advisors_handleMouse, UI_Advisors_getTooltip, UI_Advisors_init },
    { Window_SetSalaryDialog, UI_SetSalaryDialog_drawBackground, UI_SetSalaryDialog_drawForeground, UI_SetSalaryDialog_handleMouse, noop, noop },
    { Window_DonateToCityDialog, UI_DonateToCityDialog_drawBackground, UI_DonateToCityDialog_drawForeground, UI_DonateToCityDialog_handleMouse, UI_DonateToCityDialog_getTooltip, UI_DonateToCityDialog_init },
    { Window_SendGiftToCaesarDialog, UI_SendGiftToCaesarDialog_drawBackground, UI_SendGiftToCaesarDialog_drawForeground, UI_SendGiftToCaesarDialog_handleMouse, noop, UI_SendGiftToCaesarDialog_init },
    { Window_LaborPriorityDialog, UI_LaborPriorityDialog_drawBackground, UI_LaborPriorityDialog_drawForeground, UI_LaborPriorityDialog_handleMouse, UI_LaborPriorityDialog_getTooltip, noop },
    // 10
    { Window_DisplayOptions },
    { Window_SoundOptions },
    { Window_SpeedOptions },
    { Window_Empire, UI_Empire_drawBackground, UI_Empire_drawForeground, UI_Empire_handleMouse, UI_EmpireMap_getTooltip, UI_Empire_init },
    { Window_TradeOpenedDialog, UI_TradeOpenedDialog_drawBackground, UI_TradeOpenedDialog_drawForeground, UI_TradeOpenedDialog_handleMouse, noop, noop },
    { Window_HoldFestivalDialog, UI_HoldFestivalDialog_drawBackground, UI_HoldFestivalDialog_drawForeground, UI_HoldFestivalDialog_handleMouse, UI_HoldFestivalDialog_getTooltip, noop },
    { Window_TradePricesDialog, UI_TradePricesDialog_drawBackground, noop, UI_TradePricesDialog_handleMouse, UI_TradePricesDialog_getTooltip, noop },
    { Window_ResourceSettingsDialog, UI_ResourceSettingsDialog_drawBackground, UI_ResourceSettingsDialog_drawForeground, UI_ResourceSettingsDialog_handleMouse, noop, noop },
    { Window_MessageDialog, UI_MessageDialog_drawBackground, UI_MessageDialog_drawForeground, UI_MessageDialog_handleMouse, noop, UI_MessageDialog_init },
    { Window_PlayerMessageList, UI_PlayerMessageList_drawBackground, UI_PlayerMessageList_drawForeground, UI_PlayerMessageList_handleMouse, UI_PlayerMessageList_getTooltip, UI_PlayerMessageList_init },
    // 20
    { Window_CCKSelection },
    { Window_FileDialog, UI_FileDialog_drawBackground, UI_FileDialog_drawForeground, UI_FileDialog_handleMouse, noop, noop },
    { Window_OverlayMenu, UI_OverlayMenu_drawBackground, UI_OverlayMenu_drawForeground, UI_OverlayMenu_handleMouse, noop, UI_OverlayMenu_init },
    { Window_BuildingMenu, UI_BuildingMenu_drawBackground, UI_BuildingMenu_drawForeground, UI_BuildingMenu_handleMouse, noop, noop },
    { Window_Intermezzo, UI_Intermezzo_drawBackground, noop, UI_Intermezzo_handleMouse, noop, noop },
    { Window_BuildingInfo, UI_BuildingInfo_drawBackground, UI_BuildingInfo_drawForeground, UI_BuildingInfo_handleMouse, UI_BuildingInfo_getTooltip, UI_BuildingInfo_init },
    { Window_NewCareerDialog, UI_NewCareerDialog_drawBackground, UI_NewCareerDialog_drawForeground, UI_NewCareerDialog_handleMouse, noop, UI_NewCareerDialog_init },
    { Window_SlidingSidebar, UI_SlidingSidebar_drawBackground, UI_SlidingSidebar_drawForeground, noop, noop, noop },
    { Window_CityMilitary, UI_City_drawBackground, UI_City_drawForegroundMilitary, UI_City_handleMouseMilitary, UI_City_getTooltip, noop }, // TODO military command
    { Window_MissionSelection, UI_MissionStart_Selection_drawBackground, UI_MissionStart_Selection_drawForeground, UI_MissionStart_Selection_handleMouse, noop, noop },
    // 30
    { Window_MissionBriefingInitial, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingInitial_drawForeground, UI_MissionStart_BriefingInitial_handleMouse, noop, UI_MissionStart_Briefing_init },
    { Window_MissionBriefingReview, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingReview_drawForeground, UI_MissionStart_BriefingReview_handleMouse, noop, UI_MissionStart_Briefing_init },
    { Window_VictoryDialog, UI_VictoryDialog_drawBackground, UI_VictoryDialog_drawForeground, UI_VictoryDialog_handleMouse, noop, noop },
    { Window_MissionEnd, UI_MissionEnd_drawBackground, UI_MissionEnd_drawForeground, UI_MissionEnd_handleMouse, noop, noop },
    { Window_VictoryIntermezzo, noop, noop, noop, noop, UI_VictoryIntermezzo_init },
    { Window_VideoIntermezzo, UI_VideoIntermezzo_drawBackground, UI_VideoIntermezzo_drawForeground, UI_VideoIntermezzo_handleMouse, noop, UI_VideoIntermezzo_init },
    { Window_Logo },
};

static window_type previous_window;
static window_type current_window;
static int refreshRequested;

void window_invalidate()
{
    refreshRequested = 1;
}

int window_is(WindowId id)
{
    return UI_Window_getId() == id;
}

WindowId UI_Window_getId()
{
    return current_window.id;
}

void window_show(const window_type *window)
{
    previous_window = current_window;
    current_window = *window;
    if (current_window.init) {
        current_window.init();
    }
    if (!current_window.draw_background) {
        current_window.draw_background = noop;
    }
    if (!current_window.draw_foreground) {
        current_window.draw_foreground = noop;
    }
    if (!current_window.handle_mouse) {
        current_window.handle_mouse = noop;
    }
    window_invalidate();
}

void UI_Window_goTo(WindowId windowId)
{
    window_show(&windows[windowId]);
}

void UI_Window_goBack()
{
    current_window = previous_window;
    window_invalidate();
}

static void updateMouseBefore()
{
    mouse_determine_button_state();
}

static void updateMouseAfter()
{
    mouse_set_scroll(SCROLL_NONE);
    input_cursor_update(current_window.id);
}

void window_draw(int force)
{
    updateMouseBefore();
    if (force || refreshRequested) {
        current_window.draw_background();
        refreshRequested = 0;
    }
    current_window.draw_foreground();

    const mouse *m = mouse_get();
    current_window.handle_mouse(m);
    UI_Tooltip_handle(m, current_window.get_tooltip);
    UI_Warning_draw();
    updateMouseAfter();
}
