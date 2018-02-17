#include "window.h"

#include "graphics/warning.h"
#include "input/cursor.h"
#include "window/city.h"

#include "UI/Sidebar.h"

static void noop()
{
}

static window_type windows[] = {
    // 0
    { Window_MainMenu },
    { Window_City },
    { Window_PopupDialog },
    { Window_TopMenu },
    { Window_DifficultyOptions },
    { Window_Advisors },
    { Window_SetSalaryDialog },
    { Window_DonateToCityDialog },
    { Window_SendGiftToCaesarDialog },
    { Window_LaborPriorityDialog },
    // 10
    { Window_DisplayOptions },
    { Window_SoundOptions },
    { Window_SpeedOptions },
    { Window_Empire },
    { Window_TradeOpenedDialog },
    { Window_HoldFestivalDialog },
    { Window_TradePricesDialog },
    { Window_ResourceSettingsDialog },
    { Window_MessageDialog },
    { Window_PlayerMessageList },
    // 20
    { Window_CCKSelection },
    { Window_FileDialog },
    { Window_OverlayMenu },
    { Window_BuildingMenu },
    { Window_Intermezzo },
    { Window_BuildingInfo },
    { Window_NewCareerDialog },
    { Window_SlidingSidebar, UI_SlidingSidebar_drawBackground, UI_SlidingSidebar_drawForeground, noop, noop },
    { Window_CityMilitary },
    { Window_MissionSelection },
    // 30
    { Window_MissionBriefingInitial },
    { Window_MissionBriefingReview },
    { Window_VictoryDialog },
    { Window_MissionEnd },
    { Window_VideoIntermezzo },
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
    tooltip_handle(m, current_window.get_tooltip);
    warning_draw();
    updateMouseAfter();
}
