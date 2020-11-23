#ifndef BUILDING_MONUMENT_H
#define BUILDING_MONUMENT_H

#include "building/building.h"

#include "map/grid.h"
#include "map/road_access.h"
#include "translation/translation.h"
#include "window/option_popup.h"



#define MAX_MONUMENTS 30
#define MAX_MONUMENT_DELIVERIES 200
#define MONUMENT_FINISHED -1
#define MONUMENT_START 1

typedef struct {
	int walker_id;
	int destination_id;
	int resource; 
	int cartloads;
} monument_delivery;

#define MODULES_PER_TEMPLE 2
#define MARS_OFFERING_FREQUENCY 16

typedef enum {
	CERES_MODULE_1_REDUCE_FOOD,
	CERES_MODULE_2_DISTRIBUTE_FOOD,
	NEPTUNE_MODULE_1_HIPPODROME_ACCESS,
	NEPTUNE_MODULE_2_CAPACITY_AND_WATER,
	MERCURY_MODULE_1_POTTERY_FURN,
	MERCURY_MODULE_2_OIL_WINE,
	MARS_MODULE_1_MESS_HALL,
	MARS_MODULE_2_ALL_GOODS,
	VENUS_MODULE_1_DISTRIBUTE_WINE,
	VENUS_MODULE_2_DESIRABILITY_ENTERTAINMENT,
	PANTHEON_MODULE_1_DESTINATION_PRIESTS,
	PANTHEON_MODULE_2_HOUSING_EVOLUTION
} module_type;

int building_monument_access_point(building* b, map_point* dst);
int building_monument_add_module(building* b, int module_type);
int building_monument_deliver_resource(building* b, int resource);
int building_monument_get_monument(int x, int y, int resource, int road_network_id, int distance_from_entry, map_point* dst);
int building_monument_has_unfinished_monuments(void);
void building_monument_initialize(building* b);
int building_monument_is_monument(const building *b);
int building_monument_type_is_monument(building_type type);
int building_monument_type_is_mini_monument(building_type type);
int building_monument_is_grand_temple(building_type type);
int building_monument_needs_resource(building* b, int resource);
int building_monument_needs_resources(building* b);
int building_monument_progress(building* b);
void building_monument_recalculate_monuments(void);
int building_monument_working(int type);
int building_monument_resources_needed_for_monument_type(int building_type, int resource, int phase);
int building_monument_resource_in_delivery(int monument_id, int resource_id);
int building_monument_remove_delivery(int figure_id);
int building_monument_add_delivery(int monument_id, int figure_id, int resource_id, int loads_no);
int building_monument_has_monument(int type);
int building_monument_upgraded(int type);
int building_monument_module_type(int type);
int building_monument_monument_phases(int building_type);
int building_monument_gt_module_is_active(int module);
int building_monument_pantheon_module_is_active(int module);
int building_monument_finish_monuments(void);
int building_monument_phase(int phase);
int building_monument_get_venus_gt(void);
int building_monument_get_neptune_gt(void);
void building_monument_initialize_deliveries();
int building_monument_count_grand_temples();
void building_monument_delivery_save_state(buffer* list);
void building_monument_delivery_load_state(buffer* list);

#endif // BUILDING_MONUMENT_H 
