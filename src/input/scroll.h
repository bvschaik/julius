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

int scroll_get_delta(const mouse *m, pixel_offset *delta, scroll_type type);

void scroll_drag_start(int is_touch);
void scroll_drag_move(void);
int scroll_drag_end(void);

void scroll_arrow_left(int is_down);
void scroll_arrow_right(int is_down);
void scroll_arrow_up(int is_down);
void scroll_arrow_down(int is_down);

#endif // INPUT_SCROLL_H
