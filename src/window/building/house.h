#ifndef WINDOW_BUILDING_HOUSE_H
#define WINDOW_BUILDING_HOUSE_H

#include "common.h"

#include <stdint.h>

void window_building_draw_house(building_info_context *c);

const uint8_t *window_building_house_get_tooltip(const building_info_context *c);

#endif // WINDOW_BUILDING_HOUSE_H
