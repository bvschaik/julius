#include "graphics/window.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/mission_end.h"
#include "window/victory_dialog.h"

int window_is(window_id id)
{
    return id == WINDOW_CITY;
}

void window_invalidate(void)
{}

void window_logo_show(void)
{}

void window_mission_end_show_fired(void)
{}

void window_mission_end_show_won(void)
{}

void window_victory_dialog_show(void)
{}

window_id window_get_id(void)
{
    return WINDOW_CITY;
}

int window_must_refresh(void)
{
    return 0;
}

void window_draw(int force)
{}

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup)
{}

void window_popup_dialog_show(popup_dialog_type type, void (*okFunc)(int), int hasOkCancelButtons)
{}

void widget_sidebar_invalidate_minimap(void)
{}
