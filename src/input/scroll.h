#ifndef INPUT_SCROLL_H
#define INPUT_SCROLL_H

#include "city/view.h"
#include "input/mouse.h"
#include "input/touch.h"

int scroll_in_progress(void);

int scroll_get_direction(const mouse *m);

void scroll_set_custom_margins(int x, int y, int width, int height);
void scroll_restore_margins(void);

void scroll_get_delta(const mouse *m, pixel_offset *delta);

void scroll_start_touch_drag(const pixel_offset *position, touch_coords coords);
int scroll_move_touch_drag(int original_x, int original_y, int current_x, int current_y, pixel_offset *position);
void scroll_end_touch_drag(void);
int scroll_decay(pixel_offset *position);
touch_coords scroll_get_original_touch_position(void);

void scroll_arrow_left(int is_down);
void scroll_arrow_right(int is_down);
void scroll_arrow_up(int is_down);
void scroll_arrow_down(int is_down);

#endif // INPUT_SCROLL_H
