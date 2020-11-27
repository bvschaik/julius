#include "monument.h"

#include "city/finance.h"
#include "city/message.h"
#include "city/resource.h"
#include "core/calc.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/orientation.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "mods/mods.h"

#define INFINITE 10000

static int monuments[MAX_MONUMENTS] = { 0 };
static monument_delivery monument_deliveries[MAX_MONUMENT_DELIVERIES] = { 0 };

static int grand_temple_resources[6][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,20,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,8,0,20,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,8,0,16,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,16,28,12,0,0,0,},
	{4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
};
static int pantheon_resources[6][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,8,0,16,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,16,0,32,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,32,40,32,0,0,0,},
	{4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
};
static int lighthouse_resources[5][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,8,0,12,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,16,0,8,0,0,0,},
	{4,0,0,0,0,0,0,0,0,0,8,20,8,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
};
static int colosseum_resources[5][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,8,0,16,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,12,0,16,0,0,0,},
	{4,0,0,0,0,0,0,0,0,0,12,16,12,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
};
static int hippodrome_resources[5][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,16,0,32,0,0,0,},
	{1,0,0,0,0,0,0,0,0,0,16,0,32,0,0,0,},
	{4,0,0,0,0,0,0,0,0,0,32,46,32,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
};
static int oracle_lt_resources[2][RESOURCE_MAX] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
};


static struct {
	int monuments_number;
	int unfinished_monuments;
} data = { 0, 0};


int building_monument_deliver_resource(building* b, int resource) {
	if (b->id <= 0) {
		return 0;
	}
	if (!building_monument_is_monument(b)) {
		return 0;
	}
	if (b->data.monument.resources_needed[resource] <= 0) {
		return 0;
	}
	b->data.monument.resources_needed[resource]--;
	return 1;

}

int building_monument_access_point(building* b, map_point* dst)
{
	int dx = b->x - b->road_access_x;
	int dy = b->y - b->road_access_y;
	switch (b->type) {
	case BUILDING_LARGE_TEMPLE_CERES:
	case BUILDING_LARGE_TEMPLE_NEPTUNE:
	case BUILDING_LARGE_TEMPLE_MERCURY:
	case BUILDING_LARGE_TEMPLE_MARS:
	case BUILDING_LARGE_TEMPLE_VENUS:
	case BUILDING_LIGHTHOUSE:
		if (dx == -1 && dy == -3) {
			dst->x = b->x + 1;
			dst->y = b->y + 2;
		}
		else if (dx == 1 && dy == -1) {
			dst->x = b->x;
			dst->y = b->y + 1;
		}
		else if (dx == -3 && dy == -1) {
			dst->x = b->x + 2;
			dst->y = b->y + 1;
		}
		else if (dx == -1 && dy == 1) {
			dst->x = b->x + 1;
			dst->y = b->y;
		}
		else {
			return 0;
		}
		return 1;
	case BUILDING_GRAND_TEMPLE_CERES:
	case BUILDING_GRAND_TEMPLE_NEPTUNE:
	case BUILDING_GRAND_TEMPLE_MERCURY:
	case BUILDING_GRAND_TEMPLE_MARS:
	case BUILDING_GRAND_TEMPLE_VENUS:
	case BUILDING_PANTHEON:
		if (dx == -3 && dy == -7)
		{
			dst->x = b->x + 3;
			dst->y = b->y + 6;
		}
		else if (dx == 1 && dy == -3) {
			dst->x = b->x;
			dst->y = b->y + 3;
		}
		else if (dx == -3 && dy == 1) {
			dst->x = b->x + 3;
			dst->y = b->y;
		}
		else if (dx == -7 && dy == -3) {
			dst->x = b->x + 6;
			dst->y = b->y + 3;
		}
		else {
			return 0;
		}
		return 1;
	case BUILDING_COLOSSEUM:
		if (dx == -2 && dy == -5)
		{
			dst->x = b->x + 2;
			dst->y = b->y + 4;
		}
		else if (dx == 1 && dy == -2) {
			dst->x = b->x;
			dst->y = b->y + 2;
		}
		else if (dx == -2 && dy == 1) {
			dst->x = b->x + 2;
			dst->y = b->y;
		}
		else if (dx == -5 && dy == -2) {
			dst->x = b->x + 4;
			dst->y = b->y + 2;
		}
		else {
			return 0;
		}
		return 1;
	case BUILDING_ORACLE:
		dst->x = b->x;
		dst->y = b->y;	
		return 1;
	case BUILDING_HIPPODROME:
		dst->x = b->x;
		dst->y = b->y;		
		return 1;
	}
}

int building_monument_add_module(building* b, int module_type) {
	if (!building_monument_is_monument(b)) {
		return 0;
	}
	if (b->data.monument.monument_phase != MONUMENT_FINISHED) {
		return 0;
	}
	if (b->data.monument.upgrades) {
		return 0;
	}

	b->data.monument.upgrades = module_type;

	if (b->data.monument.upgrades == 1) {
		switch (b->type) {
			case BUILDING_GRAND_TEMPLE_CERES:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Module"), TERRAIN_BUILDING);
				break;
			case BUILDING_GRAND_TEMPLE_NEPTUNE:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Module"), TERRAIN_BUILDING);
				break;
			case BUILDING_GRAND_TEMPLE_MERCURY:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Module"), TERRAIN_BUILDING);
				break;
			case BUILDING_GRAND_TEMPLE_MARS:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Module"), TERRAIN_BUILDING);
				break;
			case BUILDING_GRAND_TEMPLE_VENUS:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Module"), TERRAIN_BUILDING);
				break;
			case BUILDING_PANTHEON:
				map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Module"), TERRAIN_BUILDING);
				break;
		}
	}
	else if (b->data.monument.upgrades == 2) {
		switch (b->type) {
		case BUILDING_GRAND_TEMPLE_CERES:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Module2"), TERRAIN_BUILDING);
			break;
		case BUILDING_GRAND_TEMPLE_NEPTUNE:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Module2"), TERRAIN_BUILDING);
			break;
		case BUILDING_GRAND_TEMPLE_MERCURY:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Module2"), TERRAIN_BUILDING);
			break;
		case BUILDING_GRAND_TEMPLE_MARS:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Module2"), TERRAIN_BUILDING);
			break;
		case BUILDING_GRAND_TEMPLE_VENUS:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Module2"), TERRAIN_BUILDING);
			break;
		case BUILDING_PANTHEON:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Module2"), TERRAIN_BUILDING);
			break;
		}
	}
	
	
	return 1;
}

int building_monument_get_monument(int x, int y, int resource, int road_network_id, int distance_from_entry, map_point* dst)
{
	if (city_resource_is_stockpiled(resource)) {
		return 0;
	}
	int min_dist = INFINITE;
	building* min_building = 0;
	for (int i = 0; i < data.monuments_number; i++) {
		if (!monuments[i]) {
			continue;
		}
		building* b = building_get(monuments[i]);
		if (!building_monument_is_monument(b)) {
			continue;
		}
		if (b->data.monument.monument_phase == MONUMENT_FINISHED) {
			continue;
		}
		if (!resource) {
			if (building_monument_needs_resources(b)) {
				continue;
			}
		} 
		if ((b->data.monument.resources_needed[resource] - building_monument_resource_in_delivery(b->id,resource)) <= 0) {
			continue;
		}
		if (!map_has_road_access(b->x, b->y, b->size, 0)) {
			continue;
		}
		if (b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
			continue;
		}

		int dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
		if (dist < min_dist) {
			min_dist = dist;
			min_building = b;
		}
	}
	if (min_building && min_dist < INFINITE) {
		if (dst) {
			map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
		}
		return min_building->id;
	}
	return 0;
}

int building_monument_has_unfinished_monuments() {
	return data.unfinished_monuments;
}

int building_monument_resources_needed_for_monument_type(int building_type, int resource, int phase) {
	switch (building_type)
	{
	case BUILDING_LARGE_TEMPLE_CERES:
	case BUILDING_LARGE_TEMPLE_MERCURY:
	case BUILDING_LARGE_TEMPLE_NEPTUNE:
	case BUILDING_LARGE_TEMPLE_MARS:
	case BUILDING_LARGE_TEMPLE_VENUS:
	case BUILDING_ORACLE:
		return oracle_lt_resources[phase - 1][resource];
	case BUILDING_GRAND_TEMPLE_CERES:
	case BUILDING_GRAND_TEMPLE_MERCURY:
	case BUILDING_GRAND_TEMPLE_NEPTUNE:
	case BUILDING_GRAND_TEMPLE_MARS:
	case BUILDING_GRAND_TEMPLE_VENUS:
		return grand_temple_resources[phase - 1][resource];
	case BUILDING_PANTHEON:
		return pantheon_resources[phase - 1][resource];
	case BUILDING_LIGHTHOUSE:
		return lighthouse_resources[phase - 1][resource];
	case BUILDING_COLOSSEUM:
		return colosseum_resources[phase - 1][resource];
	case BUILDING_HIPPODROME:
		return hippodrome_resources[phase - 1][resource];
	default:
		return 0;
		break;
	}
}

void building_monument_initialize(building* b)
{
	switch (b->type) {
	case BUILDING_ORACLE:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, image_group(GROUP_BUILDING_ORACLE), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_LARGE_TEMPLE_CERES:
	case BUILDING_LARGE_TEMPLE_NEPTUNE:
	case BUILDING_LARGE_TEMPLE_MERCURY:
	case BUILDING_LARGE_TEMPLE_MARS:
	case BUILDING_LARGE_TEMPLE_VENUS:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, image_group(b->type + 6) + 1, TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_GRAND_TEMPLE_CERES:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;

	case BUILDING_GRAND_TEMPLE_NEPTUNE:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;

		}
		break;
		
	case BUILDING_GRAND_TEMPLE_MERCURY:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_GRAND_TEMPLE_MARS:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_GRAND_TEMPLE_VENUS:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_PANTHEON:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 03"), TERRAIN_BUILDING);			
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 04"), TERRAIN_BUILDING);			
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 05"), TERRAIN_BUILDING);			
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon On"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_LIGHTHOUSE:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Lighthouses"), "Lighthouse Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Lighthouses"), "Lighthouse Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Lighthouses"), "Lighthouse Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Lighthouses"), "Lighthouse ON"), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	
	case BUILDING_COLOSSEUM:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Colosseum"), "Coloseum Cons 02"), TERRAIN_BUILDING);
			break;
		case 3:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Colosseum"), "Coloseum Cons 03"), TERRAIN_BUILDING);
			break;
		case 4:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Colosseum"), "Coloseum Cons 04"), TERRAIN_BUILDING);
			break;
		case 5:
			map_building_tiles_add(b->id, b->x, b->y, b->size, image_group(GROUP_BUILDING_COLOSSEUM), TERRAIN_BUILDING);
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	
	case BUILDING_HIPPODROME:
		switch (b->data.monument.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			break;
		case 2:
			map_orientation_update_buildings();
			break;
		case 3:
			map_orientation_update_buildings();
			break;
		case 4:
			map_orientation_update_buildings();
			break;
		case 5:
			map_orientation_update_buildings();
			b->data.monument.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	}

	
	if (b->data.monument.monument_phase != MONUMENT_FINISHED) {
		for (int resource = 0; resource < RESOURCE_MAX; resource++) {
			b->data.monument.resources_needed[resource] = building_monument_resources_needed_for_monument_type(b->type,resource,b->data.monument.monument_phase);
		}
	}
}

int building_monument_is_monument(const building* b) 
{
	switch (b->type) {
		case BUILDING_ORACLE:
		case BUILDING_LARGE_TEMPLE_CERES:
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
		case BUILDING_LARGE_TEMPLE_MERCURY:
		case BUILDING_LARGE_TEMPLE_MARS:
		case BUILDING_LARGE_TEMPLE_VENUS:
		case BUILDING_GRAND_TEMPLE_CERES:
		case BUILDING_GRAND_TEMPLE_NEPTUNE:
		case BUILDING_GRAND_TEMPLE_MERCURY:
		case BUILDING_GRAND_TEMPLE_MARS:
		case BUILDING_GRAND_TEMPLE_VENUS:
		case BUILDING_PANTHEON:
		case BUILDING_MENU_GRAND_TEMPLES:
		case BUILDING_LIGHTHOUSE:
		case BUILDING_COLOSSEUM:
		case BUILDING_HIPPODROME:
			return 1;
		break;
	}
	return 0;
}

int building_monument_type_is_monument(building_type type)
{
	switch (type) {
		case BUILDING_ORACLE:
		case BUILDING_LARGE_TEMPLE_CERES:
		case BUILDING_LARGE_TEMPLE_NEPTUNE:
		case BUILDING_LARGE_TEMPLE_MERCURY:
		case BUILDING_LARGE_TEMPLE_MARS:
		case BUILDING_LARGE_TEMPLE_VENUS:
		case BUILDING_GRAND_TEMPLE_CERES:
		case BUILDING_GRAND_TEMPLE_NEPTUNE:
		case BUILDING_GRAND_TEMPLE_MERCURY:
		case BUILDING_GRAND_TEMPLE_MARS:
		case BUILDING_GRAND_TEMPLE_VENUS:
		case BUILDING_PANTHEON:
		case BUILDING_MENU_GRAND_TEMPLES:
		case BUILDING_LIGHTHOUSE:
		case BUILDING_COLOSSEUM:
		case BUILDING_HIPPODROME:
			return 1;
		default:
			return 0;
	}
}

int building_monument_type_is_mini_monument(building_type type)
{
	switch (type) {
	case BUILDING_ORACLE:
	case BUILDING_LARGE_TEMPLE_CERES:
	case BUILDING_LARGE_TEMPLE_NEPTUNE:
	case BUILDING_LARGE_TEMPLE_MERCURY:
	case BUILDING_LARGE_TEMPLE_MARS:
	case BUILDING_LARGE_TEMPLE_VENUS:
		return 1;
	default:
		return 0;
	}
}

int building_monument_is_grand_temple(building_type type)
{
	switch (type) {
		case BUILDING_GRAND_TEMPLE_CERES:
		case BUILDING_GRAND_TEMPLE_NEPTUNE:
		case BUILDING_GRAND_TEMPLE_MERCURY:
		case BUILDING_GRAND_TEMPLE_MARS:
		case BUILDING_GRAND_TEMPLE_VENUS:
			return 1;
		default:
			return 0;
	}
}

int building_monument_needs_resource(building* b, int resource) {
	if (b->data.monument.monument_phase == MONUMENT_FINISHED) {
		return 0;
	}
	return (b->data.monument.resources_needed[resource]);
}


int building_monument_phase(int phase) {
	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (!building_monument_is_monument(b)) {
			continue;
		}
		b->data.monument.monument_phase = phase;
		building_monument_initialize(b);
	}
	return 1;
}

int building_monument_get_venus_gt(void) {
	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (b->type == BUILDING_GRAND_TEMPLE_VENUS) {
			return monument_id;
		}
	}
	return 0;
}

int building_monument_get_neptune_gt(void) {
	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (b->type == BUILDING_GRAND_TEMPLE_NEPTUNE) {
			return monument_id;
		}
	}
	return 0;
}

int building_monument_monument_phases(int building_type) {
	switch (building_type) {
	case BUILDING_PANTHEON:
	case BUILDING_GRAND_TEMPLE_CERES:
	case BUILDING_GRAND_TEMPLE_MERCURY:
	case BUILDING_GRAND_TEMPLE_NEPTUNE:
	case BUILDING_GRAND_TEMPLE_MARS:
	case BUILDING_GRAND_TEMPLE_VENUS:
		return 6;
	case BUILDING_LIGHTHOUSE:
	case BUILDING_COLOSSEUM:
	case BUILDING_HIPPODROME:
		return 5;
	case BUILDING_ORACLE:
	case BUILDING_LARGE_TEMPLE_CERES:
	case BUILDING_LARGE_TEMPLE_NEPTUNE:
	case BUILDING_LARGE_TEMPLE_MERCURY:
	case BUILDING_LARGE_TEMPLE_MARS:
	case BUILDING_LARGE_TEMPLE_VENUS:
		return 2;
	default:
		return 0;
		break;
	}
}

int building_monument_finish_monuments() {
	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (!building_monument_is_monument(b)) {
			continue;
		}
		b->data.monument.monument_phase = building_monument_monument_phases(b->type);
		building_monument_initialize(b);
	}
	return 1;

}

int building_monument_needs_resources(building* b) {
	if (b->data.monument.monument_phase == MONUMENT_FINISHED) {
		return 0;
	}
	for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
		if(b->data.monument.resources_needed[resource]) {
			return 1;
		}
	}
	return 0;
}

int building_monument_progress(building* b)
{
	if (building_monument_needs_resources(b)) {
		return 0;
	}
	if (b->data.monument.monument_phase == MONUMENT_FINISHED) {
		return 0;
	}

	b->data.monument.monument_phase++;
	if (b->next_part_building_id) {
		building* b2 = building_get(b->next_part_building_id);
		b2->data.monument.monument_phase++;

		building* b3 = building_get(b2->next_part_building_id);
		b3->data.monument.monument_phase++;
	}

	building_monument_initialize(b);

	if (b->data.monument.monument_phase == MONUMENT_FINISHED) {
		if (building_monument_is_grand_temple(b->type)) {
			city_message_post(1, MESSAGE_GRAND_TEMPLE_COMPLETE, 0, 0);
		}
		else if (b->type == BUILDING_PANTHEON) {
			city_message_post(1, MESSAGE_PANTHEON_COMPLETE, 0, 0);
		}
		else if (b->type == BUILDING_LIGHTHOUSE) {
			city_message_post(1, MESSAGE_LIGHTHOUSE_COMPLETE, 0, 0);
		}
		else if (b->type == BUILDING_COLOSSEUM) {
			city_message_post(1, MESSAGE_COLOSSEUM_COMPLETE, 0, 0);
		}
		else if (b->type == BUILDING_HIPPODROME) {
			city_message_post(1, MESSAGE_HIPPODROME_COMPLETE, 0, 0);
		}
	}
	return 1;
}

void building_monument_recalculate_monuments() {
	for (int i = 0; i < MAX_MONUMENTS; i++) {
		monuments[i] = 0;
	}
	data.monuments_number = 0;
	data.unfinished_monuments = 0;

	for (int i = 0; i < MAX_BUILDINGS; i++) {
		building* b = building_get(i);
		if (building_monument_is_monument(b)) {
			data.monuments_number++;
			if (b->data.monument.monument_phase != MONUMENT_FINISHED) {
				data.unfinished_monuments++;
			}
			for (int j = 0; j < MAX_MONUMENTS; j++) {
				if (monuments[j]) {
					continue;
				}
				monuments[j] = i;
				break;
			}
		}
	}
}

void building_monument_initialize_deliveries() {
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		monument_deliveries[i].destination_id = 0;
		monument_deliveries[i].walker_id = 0;
		monument_deliveries[i].resource = 0;
		monument_deliveries[i].cartloads = 0;
	}
}

int building_monument_add_delivery(int monument_id, int figure_id, int resource_id, int loads_no) {
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		if (monument_deliveries[i].destination_id == 0)
		{
			monument_deliveries[i].destination_id = monument_id;
			monument_deliveries[i].walker_id = figure_id;
			monument_deliveries[i].resource = resource_id;
			monument_deliveries[i].cartloads = loads_no;
			return 1;
		}
	}
	return 0;
}

int building_monument_remove_delivery(int figure_id) {
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		if (monument_deliveries[i].walker_id == figure_id)
		{
			monument_deliveries[i].destination_id = 0;
			monument_deliveries[i].walker_id = 0;
			monument_deliveries[i].resource = 0;
			monument_deliveries[i].cartloads = 0;
		}
	}
	return 0;
}

int building_monument_resource_in_delivery(int monument_id, int resource_id) {
	int resources = 0;
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		if (monument_deliveries[i].destination_id == monument_id && monument_deliveries[i].resource == resource_id)
		{
			resources += monument_deliveries[i].cartloads;
		}
	}
	return resources;
}

int building_monument_has_monument(int type) {

	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (b->type != type) {
			continue;
		}
		if (!building_monument_is_monument(b)) {
			return 0;
		}
		return monument_id;
	}
	return 0;
}

int building_monument_count_grand_temples() {
	int count = 0;
	for (int i = 0; i < data.monuments_number; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (building_monument_is_grand_temple(b->type)) {
			count++;
		}
	}
	return count;
}

int building_monument_working(int type) {
	int monument_id = building_monument_has_monument(type);
	building* b = building_get(monument_id);
	if (!monument_id) {
		return 0;
	}
	if (b->data.monument.monument_phase != MONUMENT_FINISHED || b->state != BUILDING_STATE_IN_USE) {
		return 0;
	}
	return monument_id;
}



int building_monument_upgraded(int type) {
	int monument_id = building_monument_working(type);
	building* b = building_get(monument_id);
	if (!monument_id) {
		return 0;
	}
	if (!b->data.monument.upgrades) {
		return 0;
	}
	return monument_id;
}

int building_monument_module_type(int type) {
	int monument_id = building_monument_working(type);
	building* b = building_get(monument_id);
	if (!monument_id) {
		return 0;
	}
	return b->data.monument.upgrades;
}

int building_monument_gt_module_is_active(int module) {
	int module_num = module % MODULES_PER_TEMPLE + 1;
	int temple_type = module / MODULES_PER_TEMPLE + BUILDING_GRAND_TEMPLE_CERES;
	
	return building_monument_module_type(temple_type) == module_num;
}

int building_monument_pantheon_module_is_active(int module) {
	return building_monument_module_type(BUILDING_PANTHEON) == (module - (PANTHEON_MODULE_1_DESTINATION_PRIESTS - 1));
}

static void delivery_save(buffer* buf, int i)
{
	buffer_write_i32(buf, monument_deliveries[i].walker_id);
	buffer_write_i32(buf, monument_deliveries[i].destination_id);
	buffer_write_i32(buf, monument_deliveries[i].resource);
	buffer_write_i32(buf, monument_deliveries[i].cartloads);
}

static void delivery_load(buffer* buf, int i)
{
	monument_deliveries[i].walker_id = buffer_read_i32(buf);
	monument_deliveries[i].destination_id = buffer_read_i32(buf);
	monument_deliveries[i].resource = buffer_read_i32(buf);
	monument_deliveries[i].cartloads = buffer_read_i32(buf);
}

void building_monument_delivery_save_state(buffer* list)
{
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		delivery_save(list,i);
	}
}

void building_monument_delivery_load_state(buffer* list)
{
	for (int i = 0; i < MAX_MONUMENT_DELIVERIES; i++) {
		delivery_load(list, i);
	}
}
