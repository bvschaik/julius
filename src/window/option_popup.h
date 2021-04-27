#ifndef WINDOW_OPTION_DIALOG_H
#define WINDOW_OPTION_DIALOG_H

typedef enum {
    OPTION_MENU_SMALL_ROW = 0,
    OPTION_MENU_LARGE_ROW = 1
} option_menu_row_size;

typedef struct {
    int header;
    int desc;
    int image_id;
} option_menu_item;

void window_option_popup_show(int title, int subtitle, option_menu_item *options, int num_options,
    void (*close_func)(int selection), int current_option, int price, option_menu_row_size row_size);

#endif // WINDOW_OPTION_DIALOG_H
