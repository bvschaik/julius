#ifndef GRAPHICS_MENU_H
#define GRAPHICS_MENU_H

#include "input/mouse.h"

#define TOP_MENU_HEIGHT 24

typedef struct {
    short text_group;
    short text_number;
    void (*left_click_handler)(int param);
    int parameter;
} menu_item;

typedef struct {
    short text_group;
    menu_item *items;
    int num_items;
    short x_start;
    short x_end;
    int calculated_width_blocks;
    int calculated_height_blocks;
} menu_bar_item;

void menu_bar_draw(menu_bar_item *items, int num_items, int max_width);
int menu_bar_handle_mouse(const mouse *m, menu_bar_item *items, int num_items, int *focus_menu_id);

void menu_draw(menu_bar_item *menu, int focus_item_id);
int menu_handle_mouse(const mouse *m, menu_bar_item *menu, int *focus_item_id);
void menu_update_text(menu_bar_item *menu, int index, int text_number);

#endif // GRAPHICS_MENU_H
