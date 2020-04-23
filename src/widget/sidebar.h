#ifndef WIDGET_SIDEBAR_H
#define WIDGET_SIDEBAR_H

#include "input/mouse.h"
#include "extra_info.h"
#include "graphics/arrow_button.h"
void widget_sidebar_draw_background(void);

void widget_sidebar_draw_foreground(void);
void widget_sidebar_draw_foreground_military(void);

int widget_sidebar_handle_mouse(const mouse *m);
int widget_sidebar_handle_mouse_build_menu(const mouse *m);

int widget_sidebar_get_tooltip_text(void);

// used in extra_info.c as well as sidebar.c
#define SIDEBAR_COLLAPSED_WIDTH 42
#define SIDEBAR_EXPANDED_WIDTH 162
#define FILLER_Y_OFFSET 474
void button_game_speed(int is_down, int param2);
static arrow_button arrow_buttons_speed[] = {
    {11, 30, 17, 24, button_game_speed, 1, 0},
    {35, 30, 15, 24, button_game_speed, 0, 0},
};

typedef struct {
    time_millis slide_start;
    int progress;
    int focus_button_for_tooltip;
    extra_info extra_info_vals; // see widget/extra_info.h for typedef
} sidebar_data;

// this global replaces the old global 'data' confined to sidebar.c;
// temporarily placing in sidebar.h while breaking out extra_info into extra_info.c
// future work will have extra_info.c functions that use sidebar_data_vals use struct pointers,
// rather than the global.
sidebar_data sidebar_data_vals;

#endif // WIDGET_SIDEBAR_H
