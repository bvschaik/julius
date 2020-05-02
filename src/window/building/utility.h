#ifndef WINDOW_BUILDING_UTILITY_H
#define WINDOW_BUILDING_UTILITY_H

#include "common.h"
#include "input/mouse.h"


void window_building_draw_engineers_post(building_info_context *c);

void window_building_draw_prefect(building_info_context *c);
void window_building_draw_roadblock(building_info_context *c);
void window_building_draw_roadblock_orders(building_info_context* c);
void window_building_draw_roadblock_orders_foreground(building_info_context* c);
void window_building_draw_roadblock_foreground(building_info_context* c);
int window_building_handle_mouse_roadblock(const mouse* m, building_info_context* c);
int window_building_handle_mouse_roadblock_orders(const mouse* m, building_info_context* c);


void window_building_draw_burning_ruin(building_info_context *c);
void window_building_draw_rubble(building_info_context *c);

void window_building_draw_reservoir(building_info_context *c);
void window_building_draw_aqueduct(building_info_context *c);
void window_building_draw_fountain(building_info_context *c);
void window_building_draw_well(building_info_context *c);

void window_building_draw_mission_post(building_info_context *c);
void window_building_draw_native_hut(building_info_context *c);
void window_building_draw_native_meeting(building_info_context *c);
void window_building_draw_native_crops(building_info_context *c);

#endif // WINDOW_BUILDING_UTILITY_H
