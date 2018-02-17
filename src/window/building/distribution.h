#ifndef WINDOW_BUILDING_DISTRIBUTION_H
#define WINDOW_BUILDING_DISTRIBUTION_H

#include "common.h"
#include "input/mouse.h"

void window_building_draw_dock(building_info_context *c);

void window_building_draw_market(building_info_context *c);

void window_building_draw_granary(building_info_context *c);
void window_building_draw_granary_foreground(building_info_context *c);
void window_building_draw_granary_orders(building_info_context *c);
void window_building_draw_granary_orders_foreground(building_info_context *c);

void window_building_handle_mouse_granary(const mouse *m, building_info_context *c);
void window_building_handle_mouse_granary_orders(const mouse *m, building_info_context *c);

void window_building_draw_warehouse(building_info_context *c);
void window_building_draw_warehouse_foreground(building_info_context *c);
void window_building_draw_warehouse_orders(building_info_context *c);
void window_building_draw_warehouse_orders_foreground(building_info_context *c);

void window_building_handle_mouse_warehouse(const mouse *m, building_info_context *c);
void window_building_handle_mouse_warehouse_orders(const mouse *m, building_info_context *c);

#endif // WINDOW_BUILDING_DISTRIBUTION_H
