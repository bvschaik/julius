#include "graphics/window.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/mission_end.h"
#include "window/victory_dialog.h"

int window_is(window_id id)
{
    return id == Window_City;
}

void window_invalidate()
{}

void window_logo_show()
{}

void window_mission_end_show_fired()
{}

void window_mission_end_show_won()
{}

void window_victory_dialog_show()
{}

window_id window_get_id()
{
    return Window_City;
}

void window_draw(int force)
{}

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup)
{}

void window_popup_dialog_show(popup_dialog_type type, void (*okFunc)(int), int hasOkCancelButtons)
{}

void widget_sidebar_invalidate_minimap()
{}
