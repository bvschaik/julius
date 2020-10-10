#include "monument.h"

#include "city/resource.h"
#include "core/calc.h"
#include "core/mods.h" 
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"



#define INFINITE 10000

static int monuments[MAX_MONUMENTS] = { 0 };
static monument_delivery monument_deliveries[MAX_MONUMENT_DELIVERIES] = { 0 };

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
	int dx = b->x-b->road_access_x;
	int dy = b->y-b->road_access_y;
	if (dx == -3 && dy == -7)
	{
		dst->x = b->x+3;
		dst->y = b->y+6;
	} else if (dx == 1 && dy == -3) {
		dst->x = b->x + 3;
		dst->y = b->y + 6;
	} else if (dx == -3 && dy == 1) {
		dst->x = b->x + 3;
		dst->y = b->y;
	} else if (dx == -7 && dy == -3) {
		dst->x = b->x + 6;
		dst->y = b->y + 3;
	} else {
		return 0;
	}
	return 1;

}

int building_monument_add_module(building* b) {
	if (!building_monument_is_monument(b)) {
		return 0;
	}
	if (b->subtype.monument_phase != MONUMENT_FINISHED) {
		return 0;
	}
	if (b->data.monument.upgrades) {
		return 0;
	}
	switch (b->type) {
	case BUILDING_GRAND_TEMPLE_CERES:
		map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Module"), TERRAIN_BUILDING);
		break;
	case BUILDING_GRAND_TEMPLE_NEPTUNE:
		map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Module2"), TERRAIN_BUILDING);
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
	b->data.monument.upgrades = 1;
	return 1;
}

int building_monument_get_monument(int x, int y, int resource, int road_network_id, int distance_from_entry, map_point* dst)
{
	if (city_resource_is_stockpiled(resource)) {
		return 0;
	}
	int min_dist = INFINITE;
	building* min_building = 0;
	for (int i = 0; i < MAX_MONUMENTS; i++) {
		if (!monuments[i]) {
			continue;
		}
		building* b = building_get(monuments[i]);
		if (!building_monument_is_monument(b)) {
			continue;
		}
		if (b->subtype.monument_phase == MONUMENT_FINISHED) {
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

void building_monument_initialize(building* b)
{

	short resources_needed[RESOURCE_MAX] = { 0 };
	switch (b->type) {
	case BUILDING_GRAND_TEMPLE_CERES:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 16;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 12;
			resources_needed[RESOURCE_TIMBER] = 20;
			resources_needed[RESOURCE_CLAY] = 36;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Ceres_Temple"), "Ceres Complex On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;

	case BUILDING_GRAND_TEMPLE_NEPTUNE:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 16;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 12;
			resources_needed[RESOURCE_TIMBER] = 20;
			resources_needed[RESOURCE_CLAY] = 36;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Neptune_Temple"), "Neptune Complex On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;

		}
		break;
		
	case BUILDING_GRAND_TEMPLE_MERCURY:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 16;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 12;
			resources_needed[RESOURCE_TIMBER] = 20;
			resources_needed[RESOURCE_CLAY] = 36;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mercury_Temple"), "Mercury Complex On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_GRAND_TEMPLE_MARS:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 16;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 12;
			resources_needed[RESOURCE_TIMBER] = 20;
			resources_needed[RESOURCE_CLAY] = 36;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Mars_Temple"), "Mars Complex On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_GRAND_TEMPLE_VENUS:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 16;
			resources_needed[RESOURCE_TIMBER] = 8;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 03"), TERRAIN_BUILDING);
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 12;
			resources_needed[RESOURCE_TIMBER] = 20;
			resources_needed[RESOURCE_CLAY] = 36;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 04"), TERRAIN_BUILDING);
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex Const 05"), TERRAIN_BUILDING);
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Venus_Temple"), "Venus Complex On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;
	case BUILDING_PANTHEON:
		switch (b->subtype.monument_phase) {
		case MONUMENT_FINISHED:
			break;
		case MONUMENT_START:
			resources_needed[RESOURCE_MARBLE] = 48;
			resources_needed[RESOURCE_NONE] = 1;
			break;
		case 2:
			resources_needed[RESOURCE_MARBLE] = 48;
			resources_needed[RESOURCE_TIMBER] = 16;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 02"), TERRAIN_BUILDING);
			break;
		case 3:
			resources_needed[RESOURCE_MARBLE] = 32;
			resources_needed[RESOURCE_TIMBER] = 16;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 03"), TERRAIN_BUILDING);			
			break;
		case 4:
			resources_needed[RESOURCE_MARBLE] = 24;
			resources_needed[RESOURCE_TIMBER] = 40;
			resources_needed[RESOURCE_CLAY] = 72;
			resources_needed[RESOURCE_NONE] = 1;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 04"), TERRAIN_BUILDING);			
			break;
		case 5:
			resources_needed[RESOURCE_NONE] = 4;
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon Const 05"), TERRAIN_BUILDING);			
			break;
		case 6:
			map_building_tiles_add(b->id, b->x, b->y, b->size, mods_get_image_id(mods_get_group_id("Areldir", "Pantheon"), "Pantheon On"), TERRAIN_BUILDING);
			b->subtype.monument_phase = MONUMENT_FINISHED;
			break;
		}
		break;

	}
	if (b->subtype.monument_phase == MONUMENT_FINISHED) {
		b->monthly_levy = 100;
	}
	for (int resource = 0; resource < RESOURCE_MAX; resource++) {
		b->data.monument.resources_needed[resource] = resources_needed[resource];
	}
}

int building_monument_is_monument(building* b) 
{
	switch (b->type) {
		case BUILDING_GRAND_TEMPLE_CERES:
		case BUILDING_GRAND_TEMPLE_NEPTUNE:
		case BUILDING_GRAND_TEMPLE_MERCURY:
		case BUILDING_GRAND_TEMPLE_MARS:
		case BUILDING_GRAND_TEMPLE_VENUS:
		case BUILDING_PANTHEON:
			return 1;
		break;
	}
	return 0;
}

int building_monument_needs_resource(building* b, int resource) {
	if (b->subtype.monument_phase == MONUMENT_FINISHED) {
		return 0;
	}
	return (b->data.monument.resources_needed[resource]);
}

int building_monument_finish_monuments() {
	for (int i = 0; i < MAX_MONUMENTS; i++) {
		int monument_id = monuments[i];
		building* b = building_get(monument_id);
		if (!building_monument_is_monument(b)) {
			continue;
		}
		b->subtype.monument_phase = 6;
		building_monument_initialize(b);
	}
	return 1;
}

int building_monument_needs_resources(building* b) {
	if (b->subtype.monument_phase == MONUMENT_FINISHED) {
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
	if (b->subtype.monument_phase == MONUMENT_FINISHED) {
		return 0;
	}


	b->subtype.monument_phase++;
	building_monument_initialize(b);
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
			if (b->subtype.monument_phase != MONUMENT_FINISHED) {
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

	for (int i = 0; i < MAX_MONUMENTS; i++) {
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

int building_monument_working(int type) {
	int monument_id = building_monument_has_monument(type);
	building* b = building_get(monument_id);
	if (!monument_id) {
		return 0;
	}
	if (b->subtype.monument_phase != MONUMENT_FINISHED || b->state != BUILDING_STATE_IN_USE) {
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