#ifndef WINDOW_ACTION_DIALOG_H
#define WINDOW_ACTION_DIALOG_H

#include "window/option_popup.h"

void window_action_popup_show(int title, int subtitle,const option_menu_item *options,
    void (*close_func)(int selection), int current_option);

#endif // WINDOW_ACTION_DIALOG_H
