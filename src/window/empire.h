#ifndef WINDOW_EMPIRE_H
#define WINDOW_EMPIRE_H

#include "empire/object.h"

void window_empire_draw_trade_waypoints(const empire_object *trade_route, int x_draw_offset, int y_draw_offset);

void window_empire_show(void);

void window_empire_show_checked(void);

#endif // WINDOW_EMPIRE_H
