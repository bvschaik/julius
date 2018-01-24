#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H

#include "input/mouse.h"

#include "UI/Tooltip.h"

typedef enum {
    Window_MainMenu,
    Window_City,
    Window_PopupDialog,
    Window_TopMenu,
    Window_DifficultyOptions,
    Window_Advisors,
    Window_SetSalaryDialog,
    Window_DonateToCityDialog,
    Window_SendGiftToCaesarDialog,
    Window_LaborPriorityDialog,
    Window_DisplayOptions,
    Window_SoundOptions,
    Window_SpeedOptions,
    Window_Empire,
    Window_TradeOpenedDialog,
    Window_HoldFestivalDialog,
    Window_TradePricesDialog,
    Window_ResourceSettingsDialog,
    Window_MessageDialog,
    Window_PlayerMessageList,
    Window_CCKSelection,
    Window_FileDialog,
    Window_OverlayMenu,
    Window_BuildingMenu,
    Window_Intermezzo,
    Window_BuildingInfo,
    Window_NewCareerDialog,
    Window_SlidingSidebar,
    Window_CityMilitary,
    Window_MissionSelection,
    Window_MissionBriefingInitial,
    Window_MissionBriefingReview,
    Window_VictoryDialog,
    Window_MissionEnd,
    Window_VideoIntermezzo,
    Window_Logo,
} WindowId;

typedef struct {
    WindowId id;
    void (*draw_background)(void);
    void (*draw_foreground)(void);
    void (*handle_mouse)(const mouse *m);
    void (*get_tooltip)(struct TooltipContext *c);
    void (*init)(void); // TODO remove later
} window_type;

void window_invalidate();

void window_draw(int force);

int window_is(WindowId id);

void window_show(const window_type *window);

WindowId UI_Window_getId();
void UI_Window_goTo(WindowId windowId);
void UI_Window_goBack();

#endif // GRAPHICS_WINDOW_H
