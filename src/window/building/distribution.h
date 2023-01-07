#ifndef WINDOW_BUILDING_DISTRIBUTION_H
#define WINDOW_BUILDING_DISTRIBUTION_H

#include "common.h"
#include "input/mouse.h"

#include <stdint.h>

void window_building_draw_dock(building_info_context *c);
void window_building_draw_dock_foreground(building_info_context* c);

int window_building_handle_mouse_dock(const mouse* m, building_info_context* c);

void window_building_draw_market(building_info_context *c);
void window_building_distributor_draw_foreground(building_info_context *c);
void window_building_draw_distributor_orders(building_info_context *c, const uint8_t *title);
void window_building_draw_distributor_orders_foreground(building_info_context *c);

int window_building_handle_mouse_distributor(const mouse* m, building_info_context* c);
int window_building_handle_mouse_distributor_orders(const mouse *m, building_info_context *c);

void window_building_draw_primary_product_stockpiling(building_info_context *c);
int window_building_handle_mouse_primary_product_producer(const mouse *m, building_info_context *c);

void window_building_draw_mess_hall(building_info_context* c);

void window_building_draw_granary(building_info_context *c);
void window_building_draw_granary_foreground(building_info_context *c);
void window_building_draw_granary_orders(building_info_context *c);
void window_building_draw_granary_orders_foreground(building_info_context *c);

int window_building_handle_mouse_granary(const mouse *m, building_info_context *c);
int window_building_handle_mouse_granary_orders(const mouse *m, building_info_context *c);

void window_building_get_tooltip_granary_orders(int *group_id, int *text_id, int *translation);

void window_building_draw_warehouse(building_info_context *c);
void window_building_draw_warehouse_foreground(building_info_context *c);
void window_building_draw_warehouse_orders(building_info_context *c);
void window_building_draw_warehouse_orders_foreground(building_info_context *c);

int window_building_handle_mouse_warehouse(const mouse *m, building_info_context *c);
int window_building_handle_mouse_warehouse_orders(const mouse *m, building_info_context *c);

void window_building_primary_product_producer_stockpiling_tooltip(int *translation);
void window_building_granary_get_tooltip_distribution_permissions(int* translation);
void window_building_warehouse_get_tooltip_distribution_permissions(int* translation);
const uint8_t *window_building_dock_get_tooltip(building_info_context *c);
void window_building_get_tooltip_warehouse_orders(int *group_id, int *text_id, int *translation);

int window_building_handle_mouse_caravanserai(const mouse *m, building_info_context *c);
void window_building_draw_caravanserai_foreground(building_info_context *c);
void window_building_draw_caravanserai(building_info_context *c);

#endif // WINDOW_BUILDING_DISTRIBUTION_H
