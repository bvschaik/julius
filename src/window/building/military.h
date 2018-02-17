#ifndef WINDOW_BUILDING_MILITARY_H
#define WINDOW_BUILDING_MILITARY_H

#include "common.h"
#include "input/mouse.h"

void window_building_draw_wall(BuildingInfoContext *c);
void window_building_draw_gatehouse(BuildingInfoContext *c);
void window_building_draw_tower(BuildingInfoContext *c);

void window_building_draw_barracks(BuildingInfoContext *c);
void window_building_draw_military_academy(BuildingInfoContext *c);

void window_building_draw_fort(BuildingInfoContext *c);

void window_building_draw_legion_info(BuildingInfoContext *c);
void window_building_draw_legion_info_foreground(BuildingInfoContext *c);
void window_building_handle_mouse_legion_info(const mouse *m, BuildingInfoContext *c);
int window_building_get_legion_info_tooltip_text(BuildingInfoContext *c);

#endif // WINDOW_BUILDING_MILITARY_H
