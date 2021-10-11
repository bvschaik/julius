#ifndef WIDGET_SCENARIO_MINIMAP_H
#define WIDGET_SCENARIO_MINIMAP_H

#include "core/buffer.h"
#include "graphics/color.h"

typedef struct {
    buffer *map_terrain;
    buffer *map_bitfields;
    buffer *map_edge;
    buffer *map_random;
    buffer *map_building;
    buffer *buildings;
    int climate;
    int grid_width;
    int grid_height;
    int grid_start;
    int grid_border_size;
} minimap_save_buffers;

void widget_scenario_minimap_draw(int x_offset, int y_offset, int width, int height);

const color_t *widget_scenario_minimap_draw_from_save(const minimap_save_buffers *buffers, int *width, int *height);

#endif // WIDGET_SCENARIO_MINIMAP_H
