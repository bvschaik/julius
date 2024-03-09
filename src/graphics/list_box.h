#ifndef LIST_BOX_H
#define LIST_BOX_H

#include "graphics/scrollbar.h"
#include "graphics/tooltip.h"
#include "input/mouse.h"

#define LIST_BOX_NO_SELECTION -1

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int index;
    int button_position;
    int is_selected;
    int is_focused;
} list_box_item;

typedef struct {
    int x;
    int y;
    int width_blocks;
    int height_blocks;
    int item_height;
    int draw_inner_panel;
    int extend_to_hidden_scrollbar;
    int decorate_scrollbar;
    void (*draw_item)(const list_box_item *item);
    void (*on_select)(int index, int is_double_click);
    void (*handle_tooltip)(const list_box_item *item, tooltip_context *c);

    /* Private elements */
    int total_items;
    int selected_index;
    scrollbar_type scrollbar;
    int focus_button_id;
    int refresh_requested;
} list_box_type;

void list_box_init(list_box_type *list_box, int total_items);
void list_box_update_total_items(list_box_type *list_box, int total_items);
int list_box_get_total_items(const list_box_type *list_box);
void list_box_draw(list_box_type *list_box);
int list_box_handle_input(list_box_type *list_box, const mouse *m, int in_dialog);
void list_box_handle_tooltip(const list_box_type *list_box, tooltip_context *c);
void list_box_show_index(list_box_type *list_box, int index);
void list_box_show_selected_index(list_box_type *list_box);
void list_box_select_index(list_box_type *list_box, int index);
int list_box_get_selected_index(const list_box_type *list_box);
void list_box_request_refresh(list_box_type *list_box);
int list_box_get_scroll_position(const list_box_type *list_box);

#endif // LIST_BOX_H
