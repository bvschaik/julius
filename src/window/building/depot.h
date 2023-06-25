#ifndef WINDOW_BUILDING_DEPOT_H
#define WINDOW_BUILDING_DEPOT_H

#include "common.h"
#include "input/mouse.h"

void window_building_depot_init_main(int building_id);
void window_building_depot_init_resource_selection(void);
void window_building_depot_init_storage_selection(void);

void window_building_draw_depot(building_info_context *c);
void window_building_draw_depot_foreground(building_info_context *c);
void window_building_draw_depot_select_resource(building_info_context *c);
void window_building_draw_depot_select_resource_foreground(building_info_context *c);
void window_building_draw_depot_select_source_destination(building_info_context *c);
void window_building_draw_depot_order_source_destination_background(building_info_context *c, int is_select_destination);

int window_building_handle_mouse_depot(const mouse *m, building_info_context *c);
int window_building_handle_mouse_depot_select_source(const mouse *m, building_info_context *c);
int window_building_handle_mouse_depot_select_destination(const mouse *m, building_info_context *c);
int window_building_handle_mouse_depot_select_resource(const mouse *m, building_info_context *c);

void window_building_depot_get_tooltip_main(int *translation);
void window_building_depot_get_tooltip_source_destination(int *translation);

#endif // WINDOW_BUILDING_DEPOT_H
