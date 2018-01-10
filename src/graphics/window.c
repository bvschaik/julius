#include "window.h"

#include "input/cursor.h"

#include "UI/AllWindows.h"
#include "UI/BuildingInfo.h"
#include "UI/FileDialog.h"
#include "UI/PopupDialog.h"
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
    { Window_MainMenu, noop, UI_MainMenu_drawBackground, UI_MainMenu_drawForeground, UI_MainMenu_handleMouse },
    { Window_City, noop, UI_City_drawBackground, UI_City_drawForeground, UI_City_handleMouse, UI_City_getTooltip },
    { Window_PopupDialog, noop, UI_PopupDialog_drawBackground, UI_PopupDialog_drawForeground, UI_PopupDialog_handleMouse },
    { Window_TopMenu, noop, UI_City_drawBackground, UI_TopMenu_drawForeground, UI_TopMenu_handleMouse },
    { Window_DifficultyOptions, noop, noop, UI_DifficultyOptions_drawForeground, UI_DifficultyOptions_handleMouse },
    { Window_Advisors, UI_Advisors_init, UI_Advisors_drawBackground, UI_Advisors_drawForeground, UI_Advisors_handleMouse, UI_Advisors_getTooltip },
    { Window_SetSalaryDialog, noop, UI_SetSalaryDialog_drawBackground, UI_SetSalaryDialog_drawForeground, UI_SetSalaryDialog_handleMouse },
    { Window_DonateToCityDialog, UI_DonateToCityDialog_init, UI_DonateToCityDialog_drawBackground, UI_DonateToCityDialog_drawForeground, UI_DonateToCityDialog_handleMouse, UI_DonateToCityDialog_getTooltip },
    { Window_SendGiftToCaesarDialog, UI_SendGiftToCaesarDialog_init, UI_SendGiftToCaesarDialog_drawBackground, UI_SendGiftToCaesarDialog_drawForeground, UI_SendGiftToCaesarDialog_handleMouse },
    { Window_LaborPriorityDialog, noop, UI_LaborPriorityDialog_drawBackground, UI_LaborPriorityDialog_drawForeground, UI_LaborPriorityDialog_handleMouse, UI_LaborPriorityDialog_getTooltip },
    // 10
    { Window_DisplayOptions, noop, noop, UI_DisplayOptions_drawForeground, UI_DisplayOptions_handleMouse },
    { Window_SoundOptions, UI_SoundOptions_init, noop, UI_SoundOptions_drawForeground, UI_SoundOptions_handleMouse },
    { Window_SpeedOptions, UI_SpeedOptions_init, noop, UI_SpeedOptions_drawForeground, UI_SpeedOptions_handleMouse },
    { Window_Empire, UI_Empire_init, UI_Empire_drawBackground, UI_Empire_drawForeground, UI_Empire_handleMouse, UI_EmpireMap_getTooltip },
    { Window_TradeOpenedDialog, noop, UI_TradeOpenedDialog_drawBackground, UI_TradeOpenedDialog_drawForeground, UI_TradeOpenedDialog_handleMouse },
    { Window_HoldFestivalDialog, noop, UI_HoldFestivalDialog_drawBackground, UI_HoldFestivalDialog_drawForeground, UI_HoldFestivalDialog_handleMouse, UI_HoldFestivalDialog_getTooltip },
    { Window_TradePricesDialog, noop, UI_TradePricesDialog_drawBackground, noop, UI_TradePricesDialog_handleMouse, UI_TradePricesDialog_getTooltip },
    { Window_ResourceSettingsDialog, noop, UI_ResourceSettingsDialog_drawBackground, UI_ResourceSettingsDialog_drawForeground, UI_ResourceSettingsDialog_handleMouse },
    { Window_MessageDialog, UI_MessageDialog_init, UI_MessageDialog_drawBackground, UI_MessageDialog_drawForeground, UI_MessageDialog_handleMouse },
    { Window_PlayerMessageList, UI_PlayerMessageList_init, UI_PlayerMessageList_drawBackground, UI_PlayerMessageList_drawForeground, UI_PlayerMessageList_handleMouse, UI_PlayerMessageList_getTooltip },
    // 20
    { Window_CCKSelection, UI_CCKSelection_init, UI_CCKSelection_drawBackground, UI_CCKSelection_drawForeground, UI_CCKSelection_handleMouse},
    { Window_FileDialog, noop, UI_FileDialog_drawBackground, UI_FileDialog_drawForeground, UI_FileDialog_handleMouse },
    { Window_OverlayMenu, UI_OverlayMenu_init, UI_OverlayMenu_drawBackground, UI_OverlayMenu_drawForeground, UI_OverlayMenu_handleMouse },
    { Window_BuildingMenu, noop, UI_BuildingMenu_drawBackground, UI_BuildingMenu_drawForeground, UI_BuildingMenu_handleMouse },
    { Window_Intermezzo, noop, UI_Intermezzo_drawBackground, noop, UI_Intermezzo_handleMouse },
    { Window_BuildingInfo, UI_BuildingInfo_init, UI_BuildingInfo_drawBackground, UI_BuildingInfo_drawForeground, UI_BuildingInfo_handleMouse, UI_BuildingInfo_getTooltip },
    { Window_NewCareerDialog, UI_NewCareerDialog_init, UI_NewCareerDialog_drawBackground, UI_NewCareerDialog_drawForeground, UI_NewCareerDialog_handleMouse },
    { Window_SlidingSidebar, noop, UI_SlidingSidebar_drawBackground, UI_SlidingSidebar_drawForeground, noop },
    { Window_CityMilitary, noop, UI_City_drawBackground, UI_City_drawForegroundMilitary, UI_City_handleMouseMilitary, UI_City_getTooltip }, // TODO military command
    { Window_MissionSelection, noop, UI_MissionStart_Selection_drawBackground, UI_MissionStart_Selection_drawForeground, UI_MissionStart_Selection_handleMouse },
    // 30
    { Window_MissionBriefingInitial, UI_MissionStart_Briefing_init, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingInitial_drawForeground, UI_MissionStart_BriefingInitial_handleMouse },
    { Window_MissionBriefingReview, UI_MissionStart_Briefing_init, UI_MissionStart_Briefing_drawBackground, UI_MissionStart_BriefingReview_drawForeground, UI_MissionStart_BriefingReview_handleMouse },
    { Window_VictoryDialog, noop, UI_VictoryDialog_drawBackground, UI_VictoryDialog_drawForeground, UI_VictoryDialog_handleMouse },
    { Window_MissionEnd, noop, UI_MissionEnd_drawBackground, UI_MissionEnd_drawForeground, UI_MissionEnd_handleMouse },
    { Window_VictoryIntermezzo, UI_VictoryIntermezzo_init, noop, noop, noop },
    { Window_VideoIntermezzo, UI_VideoIntermezzo_init, UI_VideoIntermezzo_drawBackground, UI_VideoIntermezzo_drawForeground, UI_VideoIntermezzo_handleMouse },
    { Window_Logo, UI_Logo_init, UI_Logo_drawBackground, noop, UI_Logo_handleMouse },
};

static const window_type *previous_window;
static const window_type *current_window;
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
    return current_window->id;
}

void window_show(const window_type *window)
{
    previous_window = current_window;
    current_window = window;
    current_window->init();
    window_invalidate();
}

void UI_Window_goTo(WindowId windowId)
{
    window_show(&windows[windowId]);
}

void UI_Window_goBack()
{
    window_show(previous_window);
}

static void updateMouseBefore()
{
    mouse_determine_button_state();
}

static void updateMouseAfter()
{
    mouse_set_scroll(SCROLL_NONE);
    input_cursor_update(current_window->id);
}

void window_draw(int force)
{
    updateMouseBefore();
    if (force || refreshRequested) {
        current_window->draw_background();
        refreshRequested = 0;
    }
    current_window->draw_foreground();

    const mouse *m = mouse_get();
    current_window->handle_mouse(m);
    UI_Tooltip_handle(m, current_window->get_tooltip);
    UI_Warning_draw();
    updateMouseAfter();
}
