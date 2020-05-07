#include "graphics/window.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/mission_end.h"
#include "window/victory_dialog.h"

#include "city/victory.h"

int window_is(window_id id)
{
    return id == WINDOW_CITY;
}

void window_invalidate(void)
{}

void window_request_refresh(void)
{}

void window_logo_show(int show_patch_message)
{}

void window_main_menu_show(int restart_music)
{}

void window_mission_end_show_fired(void)
{}

void window_mission_end_show_won(void)
{}

void window_victory_dialog_show(void)
{
    city_victory_continue_governing(60);
    city_victory_reset();
}

void window_editor_map_show(void)
{}

window_id window_get_id(void)
{
    return WINDOW_CITY;
}

int window_is_invalid(void)
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

void widget_minimap_invalidate(void)
{}

int window_building_info_get_building_type(void)
{
    return 0;
}
