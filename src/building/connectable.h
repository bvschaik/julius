#ifndef BUILDING_CONNECTABLE_H
#define BUILDING_CONNECTABLE_H

#include "building/type.h"

#define BUILDING_CONNECTABLE_ROTATION_LIMIT_HEDGES 3
#define BUILDING_CONNECTABLE_ROTATION_LIMIT_PATHS 2

enum {
    CONTEXT_HEDGES,
    CONTEXT_COLONNADE,
    CONTEXT_GARDEN_TREE_PATH,
    CONTEXT_GARDEN_PATH_INTERSECTION,
    CONTEXT_GARDEN_TREELESS_PATH,
    CONTEXT_GARDEN_WALLS,
    CONTEXT_GARDEN_GATE,
    CONTEXT_PALISADES,
    CONTEXT_MAX_ITEMS
};

int building_connectable_gate_type(building_type type);

int building_connectable_get_hedge_offset(int grid_offset);
int building_connectable_get_hedge_gate_offset(int grid_offset);
int building_connectable_get_colonnade_offset(int grid_offset);
int building_connectable_get_garden_wall_offset(int grid_offset);
int building_connectable_get_garden_path_offset(int grid_offset, int context);
int building_connectable_get_garden_gate_offset(int grid_offset);
int building_connectable_get_palisade_offset(int grid_offset);
int building_connectable_get_palisade_gate_offset(int grid_offset);


int building_is_connectable(building_type type);
int building_connectable_num_variants(building_type type);

void building_connectable_update_connections(void);
void building_connectable_update_connections_for_type(building_type type);


#endif // BUILDING_CONNECTABLE_H
