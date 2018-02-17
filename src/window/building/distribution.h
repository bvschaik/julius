#ifndef WINDOW_BUILDING_DISTRIBUTION_H
#define WINDOW_BUILDING_DISTRIBUTION_H

#include "common.h"
#include "input/mouse.h"

void window_building_draw_dock(BuildingInfoContext *c);

void window_building_draw_market(BuildingInfoContext *c);

void window_building_draw_granary(BuildingInfoContext *c);
void window_building_draw_granary_foreground(BuildingInfoContext *c);
void window_building_draw_granary_orders(BuildingInfoContext *c);
void window_building_draw_granary_orders_foreground(BuildingInfoContext *c);

void window_building_handle_mouse_granary(const mouse *m, BuildingInfoContext *c);
void window_building_handle_mouse_granary_orders(const mouse *m, BuildingInfoContext *c);

void window_building_draw_warehouse(BuildingInfoContext *c);
void window_building_draw_warehouse_foreground(BuildingInfoContext *c);
void window_building_draw_warehouse_orders(BuildingInfoContext *c);
void window_building_draw_warehouse_orders_foreground(BuildingInfoContext *c);

void window_building_handle_mouse_warehouse(const mouse *m, BuildingInfoContext *c);
void window_building_handle_mouse_warehouse_orders(const mouse *m, BuildingInfoContext *c);

#endif // WINDOW_BUILDING_DISTRIBUTION_H
