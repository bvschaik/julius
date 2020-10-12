#ifndef BUILDING_MONUMENT_H
#define BUILDING_MONUMENT_H

#include "building/building.h"

#include "map/grid.h"
#include "map/road_access.h"
#include "translation/translation.h"
#include "window/option_popup.h"



#define MAX_MONUMENTS 10
#define MAX_MONUMENT_DELIVERIES 200
#define MONUMENT_FINISHED -1
#define MONUMENT_START 1

typedef struct {
	int walker_id;
	int destination_id;
	int resource; 
	int cartloads;
} monument_delivery;

int building_monument_access_point(building* b, map_point* dst);
int building_monument_add_module(building* b, int module_type);
int building_monument_deliver_resource(building* b, int resource);
int building_monument_get_monument(int x, int y, int resource, int road_network_id, int distance_from_entry, map_point* dst);
int building_monument_has_unfinished_monuments(void);
void building_monument_initialize(building* b);
int building_monument_is_monument(building* b);
int building_monument_needs_resource(building* b, int resource);
int building_monument_needs_resources(building* b);
int building_monument_progress(building* b);
void building_monument_recalculate_monuments(void);
int building_monument_working(int type);
int building_monument_resource_in_delivery(int monument_id, int resource_id);
int building_monument_remove_delivery(int figure_id);
int building_monument_add_delivery(int monument_id, int figure_id, int resource_id, int loads_no);
int building_monument_has_monument(int type);
int building_monument_upgraded(int type);
int building_monument_finish_monuments(void);
int building_monument_phase(int phase);





#endif // BUILDING_MONUMENT_H 
