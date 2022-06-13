#ifndef WIDGET_MINIMAP_H
#define WIDGET_MINIMAP_H

#include "building/building.h"
#include "figure/figure.h"
#include "input/mouse.h"
#include "scenario/property.h"

typedef struct {
    scenario_climate(*climate)(void);
    building *(*building)(int id);
    struct {
        int (*width)(void);
        int (*height)(void);
    } map;
    struct {
        int (*figure)(int grid_offset, int (*callback)(figure *f));
        int (*terrain)(int grid_offset);
        int (*building_id)(int grid_offset);
        int (*is_draw_tile)(int grid_offset);
        int (*tile_size)(int grid_offset);
        int (*random)(int grid_offset);
    } offset;
    void (*viewport)(int *x, int *y, int *width, int *height);
} minimap_functions;

void widget_minimap_invalidate(void);

void widget_minimap_update(const minimap_functions *functions);

void widget_minimap_draw(int x_offset, int y_offset, int width, int height);

void widget_minimap_draw_decorated(int x_offset, int y_offset, int width, int height);

int widget_minimap_handle_mouse(const mouse *m);

#endif // WIDGET_MINIMAP_H
