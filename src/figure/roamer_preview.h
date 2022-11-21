#ifndef FIGURE_ROAMER_PREVIEW_H
#define FIGURE_ROAMER_PREVIEW_H

#include "building/building.h"
#include "map/point.h"

#define FIGURE_ROAMER_PREVIEW_MAX_PASSAGES 8
#define FIGURE_ROAMER_PREVIEW_ENTRY_TILE 9
#define FIGURE_ROAMER_PREVIEW_EXIT_TILE 10
#define FIGURE_ROAMER_PREVIEW_ENTRY_EXIT_TILE 11

void figure_roamer_preview_create(building_type b_type, int grid_offset, int x, int y);
void figure_roamer_preview_create_all_for_building_type(building_type type);
void figure_roamer_preview_reset(building_type type);
void figure_roamer_preview_reset_building_types(void);
int figure_roamer_preview_get_frequency(int grid_offset);

#endif // FIGURE_ROAMER_PREVIEW_H
