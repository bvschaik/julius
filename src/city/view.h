#ifndef CITY_VIEW_H
#define CITY_VIEW_H

#include "core/buffer.h"

int city_view_orientation();

void city_view_save_state(buffer *orientation, buffer *camera);

void city_view_load_state(buffer *orientation, buffer *camera);

void city_view_load_scenario_state(buffer *camera);

#endif // CITY_VIEW_H
