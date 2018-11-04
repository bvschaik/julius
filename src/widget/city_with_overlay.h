#ifndef WIDGET_CITY_WITH_OVERLAY_H
#define WIDGET_CITY_WITH_OVERLAY_H

#include "graphics/tooltip.h"
#include "map/point.h"

void city_with_overlay_draw(const map_tile *tile);

int city_with_overlay_get_tooltip_text(tooltip_context *c, int grid_offset);

#endif // WIDGET_CITY_WITH_OVERLAY_H
