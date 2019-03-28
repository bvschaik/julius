#ifndef GRAPHICS_MENU_H
#define GRAPHICS_MENU_H

#include "input/mouse.h"

typedef struct {
    short y_start;
    short text_number;
    void (*left_click_handler)(int param);
    int parameter;
} menu_item;

typedef struct {
    short x_start;
    short x_end;
    short y_start;
    short text_group;
    menu_item *items;
    int num_items;
    int calculated_width_blocks;
} menu_bar_item;

void menu_bar_draw(menu_bar_item *items, int num_items);
int menu_bar_handle_mouse(const mouse *m, menu_bar_item *items, int num_items, int *focus_menu_id);

void menu_draw(menu_bar_item *menu, int focus_item_id);
int menu_handle_mouse(const mouse *m, menu_bar_item *menu, int *focus_item_id);
void menu_update_text(menu_bar_item *menu, int index, int text_number);

#endif // GRAPHICS_MENU_H
