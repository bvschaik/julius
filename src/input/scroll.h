#ifndef INPUT_SCROLL_H
#define INPUT_SCROLL_H

#include "city/view.h"
#include "input/mouse.h"
#include "input/touch.h"

typedef enum {
    SCROLL_TYPE_CITY = 0,
    SCROLL_TYPE_EMPIRE = 1,
    SCROLL_TYPE_MAX = 2
} scroll_type;

int scroll_in_progress(void);
int scroll_is_smooth(void);

void scroll_set_custom_margins(int x, int y, int width, int height);
void scroll_restore_margins(void);

void scroll_get_delta(const mouse *m, pixel_offset *delta, scroll_type type);

void scroll_start_mouse_drag(const mouse *m, const pixel_offset *position);
int scroll_move_mouse_drag(const mouse *m, pixel_offset *position);
int scroll_end_mouse_drag(void);

void scroll_start_touch_drag(const pixel_offset *position, touch_coords coords);
int scroll_move_touch_drag(int original_x, int original_y, int current_x, int current_y, pixel_offset *position);
void scroll_end_touch_drag(int do_decay);
int scroll_decay(pixel_offset *position);
touch_coords scroll_get_original_touch_position(void);

void scroll_arrow_left(int is_down);
void scroll_arrow_right(int is_down);
void scroll_arrow_up(int is_down);
void scroll_arrow_down(int is_down);

#endif // INPUT_SCROLL_H
