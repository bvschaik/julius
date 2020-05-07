#ifndef INPUT_ZOOM_H
#define INPUT_ZOOM_H

#include "city/view.h"
#include "input/mouse.h"
#include "input/touch.h"

void zoom_map(const mouse *m);
void zoom_update_touch(const touch *first, const touch *last, int scale);
void zoom_end_touch(void);

int zoom_update_value(int *zoom, pixel_offset *camera_position);

#endif // INPUT_ZOOM_H
