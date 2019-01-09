#ifndef GRAPHICS_WINDOW_H
#define GRAPHICS_WINDOW_H

#include "graphics/tooltip.h"
#include "input/mouse.h"

typedef enum {
    WINDOW_LOGO,
    WINDOW_MAIN_MENU,
    WINDOW_NEW_CAREER,
    WINDOW_CCK_SELECTION,
    WINDOW_FILE_DIALOG,
    WINDOW_POPUP_DIALOG,
    // mission start/end
    WINDOW_INTERMEZZO,
    WINDOW_MISSION_SELECTION,
    WINDOW_MISSION_BRIEFING,
    WINDOW_VICTORY_DIALOG,
    WINDOW_VICTORY_VIDEO,
    WINDOW_MISSION_END,
    // city
    WINDOW_CITY,
    WINDOW_CITY_MILITARY,
    WINDOW_TOP_MENU,
    WINDOW_OVERLAY_MENU,
    WINDOW_BUILD_MENU,
    WINDOW_SLIDING_SIDEBAR,
    WINDOW_MESSAGE_DIALOG,
    WINDOW_MESSAGE_LIST,
    WINDOW_BUILDING_INFO,
    // advisors and dialogs
    WINDOW_ADVISORS,
    WINDOW_LABOR_PRIORITY,
    WINDOW_SET_SALARY,
    WINDOW_DONATE_TO_CITY,
    WINDOW_GIFT_TO_EMPEROR,
    WINDOW_TRADE_PRICES,
    WINDOW_RESOURCE_SETTINGS,
    WINDOW_HOLD_FESTIVAL,
    // empire and dialog
    WINDOW_EMPIRE,
    WINDOW_TRADE_OPENED,
    // options dialogs
    WINDOW_DIFFICULTY_OPTIONS,
    WINDOW_DISPLAY_OPTIONS,
    WINDOW_SOUND_OPTIONS,
    WINDOW_SPEED_OPTIONS,
} window_id;

typedef struct {
    window_id id;
    void (*draw_background)(void);
    void (*draw_foreground)(void);
    void (*handle_mouse)(const mouse *m);
    void (*get_tooltip)(tooltip_context *c);
} window_type;

void window_invalidate(void);

void window_request_refresh_on_draw(void);

int window_must_refresh(void);

void window_draw(int force);

int window_is(window_id id);

void window_show(const window_type *window);

window_id window_get_id(void);

void window_go_back(void);

#endif // GRAPHICS_WINDOW_H
