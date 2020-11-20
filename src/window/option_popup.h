#ifndef WINDOW_OPTION_DIALOG_H
#define WINDOW_OPTION_DIALOG_H

#include <stdint.h>

typedef struct {
    int show;
    int header;
    int desc;
    int image_id;
    uint8_t mod_author[24];
    uint8_t mod_name[24];
    uint8_t mod_image_id[24];
    int required_allowed_building_id;
} option_menu_item;

void window_option_popup_show(int title, int subtitle, option_menu_item *options,
    void (*close_func)(int selection), int show_cancel_button);



#endif // WINDOW_OPTION_DIALOG_H
