#ifndef INPUT_TOUCH_H
#define INPUT_TOUCH_H

#include "core/time.h"

#define MAX_ACTIVE_TOUCHES 2

typedef struct {
    int x;
    int y;
} touch_coords;

typedef struct {
    float x;
    float y;
} touch_speed;

typedef struct {
    int in_use;
    int has_started;
    int has_moved;
    int has_ended;
    touch_coords start_point;
    touch_coords current_point;
    touch_coords frame_movement;
    touch_speed speed;
    time_millis start_time;
    time_millis last_change_time;
} touch;

const touch *get_earliest_touch(void);
const touch *get_latest_touch(void);
int get_total_active_touches(void);

int touch_not_click(const touch *t);
int touch_was_click(const touch *t);
int touch_was_double_click(const touch *t);

int touch_is_scroll(void);
int touch_get_scroll(void);

void reset_touches(int reset_old_touch);

int touch_create(touch_coords start_coords, time_millis start_time);
void touch_update(int index, touch_coords current_coords, touch_coords frame_movement, time_millis current_time, int has_ended);

int touch_to_mouse(void);

#endif // INPUT_TOUCH_H
