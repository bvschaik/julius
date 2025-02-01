#ifndef GRID_BOX_H
#define GRID_BOX_H

#include "graphics/scrollbar.h"
#include "graphics/tooltip.h"
#include "input/mouse.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    unsigned int index;
    unsigned int position;
    int is_focused;
    struct {
        unsigned int x;
        unsigned int y;
    } mouse;
} grid_box_item;

typedef struct {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned int item_height;
    unsigned int num_columns;
    struct {
        int horizontal;
        int vertical;
    } item_margin;
    int draw_inner_panel;
    int extend_to_hidden_scrollbar;
    int decorate_scrollbar;
    void (*draw_item)(const grid_box_item *item);
    void (*on_click)(const grid_box_item *item);
    void (*handle_tooltip)(const grid_box_item *item, tooltip_context *c);

    /* Private elements */
    unsigned int total_items;
    grid_box_item focused_item;
    scrollbar_type scrollbar;
    int refresh_requested;
} grid_box_type;

void grid_box_init(grid_box_type *grid_box, unsigned int total_items);
void grid_box_update_total_items(grid_box_type *grid_box, unsigned int total_items);
unsigned int grid_box_get_total_items(const grid_box_type *grid_box);
void grid_box_draw(grid_box_type *grid_box);
int grid_box_handle_input(grid_box_type *grid_box, const mouse *m, int in_dialog);
void grid_box_handle_tooltip(const grid_box_type *grid_box, tooltip_context *c);
void grid_box_show_index(grid_box_type *grid_box, unsigned int index);
void grid_box_request_refresh(grid_box_type *grid_box);
int grid_box_has_scrollbar(const grid_box_type *grid_box);
unsigned int grid_box_get_scroll_position(const grid_box_type *grid_box);

#endif // GRID_BOX_H
