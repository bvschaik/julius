#ifndef BUILDING_ROADBLOCK_H
#define BUILDING_ROADBLOCK_H

#include "building/building.h"

typedef enum {
	PERMISSION_NONE = 0,
	PERMISSION_MAINTENANCE = 1,
	PERMISSION_PRIEST = 2,
	PERMISSION_MARKET = 3,
	PERMISSION_ENTERTAINER = 4,
} roadblock_permission;

void building_roadblock_set_permission(roadblock_permission p, building* b);
int building_roadblock_get_permission(roadblock_permission p, building* b);

#endif // BUILDING_ROADBLOCK_H
