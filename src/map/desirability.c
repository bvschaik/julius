#include "desirability.h"

#include "building/model.h"
#include "core/calc.h"
#include "map/grid.h"
#include "map/ring.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/State.h"
#include "Terrain.h"

void map_desirability_clear()
{
    map_grid_clear_i8(Data_Grid_desirability);
}

static void add_desirability_at_distance(int x, int y, int size, int distance, int desirability)
{
    int partially_outside_map = 0;
    if (x - distance < -1 || x + distance + size - 1 > Data_State.map.width) {
        partially_outside_map = 1;
    }
    if (y - distance < -1 || y + distance + size - 1 > Data_State.map.height) {
        partially_outside_map = 1;
    }
    int base_offset = map_grid_offset(x, y);
    int start = map_ring_start(size, distance);
    int end = map_ring_end(size, distance);

    if (partially_outside_map) {
        for (int i = start; i < end; i++) {
            const ring_tile *tile = map_ring_tile(i);
            if (map_ring_is_inside_map(x + tile->x, y + tile->y)) {
                Data_Grid_desirability[base_offset + tile->grid_offset] += desirability;
                Data_Grid_desirability[base_offset] = calc_bound(Data_Grid_desirability[base_offset], -100, 100); // BUG: bounding on wrong tile
            }
        }
    } else {
        for (int i = start; i < end; i++) {
            const ring_tile *tile = map_ring_tile(i);
            Data_Grid_desirability[base_offset + tile->grid_offset] =
                calc_bound(Data_Grid_desirability[base_offset + tile->grid_offset] + desirability, -100, 100);
        }
    }
}

static void add_to_terrain(int x, int y, int size, int desirability, int step, int step_size, int range)
{
    if (size > 0) {
        if (range > 6) range = 6;
        int tiles_within_step = 0;
        int distance = 1;
        while (range > 0) {
            add_desirability_at_distance(x, y, size, distance, desirability);
            distance++;
            range--;
            tiles_within_step++;
            if (tiles_within_step >= step) {
                desirability += step_size;
                tiles_within_step = 0;
            }
        }
    }
}

static void update_buildings()
{
    for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
        if (BuildingIsInUse(i)) {
            const model_building *model = model_get_building(Data_Buildings[i].type);
            add_to_terrain(
                Data_Buildings[i].x, Data_Buildings[i].y,
                Data_Buildings[i].size,
                model->desirability_value,
                model->desirability_step,
                model->desirability_step_size,
                model->desirability_range);
        }
    }
}

static void update_terrain()
{
	int grid_offset = Data_State.map.gridStartOffset;
	for (int y = 0; y < Data_State.map.height; y++, grid_offset += Data_State.map.gridBorderSize) {
		for (int x = 0; x < Data_State.map.width; x++, grid_offset++) {
			int terrain = Data_Grid_terrain[grid_offset];
			if (Data_Grid_bitfields[grid_offset] & Bitfield_PlazaOrEarthquake) {
				int type;
				if (terrain & Terrain_Road) {
					type = BUILDING_PLAZA;
				} else if (terrain & Terrain_Rock) {
					// earthquake fault line: slight negative
					type = BUILDING_HOUSE_VACANT_LOT;
				} else {
					// invalid plaza/earthquake flag
					Data_Grid_bitfields[grid_offset] &= ~Bitfield_PlazaOrEarthquake;
					continue;
				}
				const model_building *model = model_get_building(type);
				add_to_terrain(x, y, 1,
                    model->desirability_value,
                    model->desirability_step,
                    model->desirability_step_size,
                    model->desirability_range);
			} else if (terrain & Terrain_Garden) {
                const model_building *model = model_get_building(BUILDING_GARDENS);
                add_to_terrain(x, y, 1,
                    model->desirability_value,
                    model->desirability_step,
                    model->desirability_step_size,
                    model->desirability_range);
			} else if (terrain & Terrain_Rubble) {
				add_to_terrain(x, y, 1, -2, 1, 1, 2);
			}
		}
	}
}

void map_desirability_update()
{
    map_desirability_clear();
    update_buildings();
    update_terrain();
}

int map_desirability_get(int grid_offset)
{
    return Data_Grid_desirability[grid_offset];
}

int map_desirability_get_max(int x, int y, int size)
{
    if (size == 1) {
        return Data_Grid_desirability[map_grid_offset(x, y)];
    }
    int max = -9999;
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if (Data_Grid_desirability[grid_offset] > max) {
                max = Data_Grid_desirability[grid_offset];
            }
        }
    }
    return max;
}

void map_desirability_save_state(buffer *buf)
{
    map_grid_save_state_i8(Data_Grid_desirability, buf);
}

void map_desirability_load_state(buffer *buf)
{
    map_grid_load_state_i8(Data_Grid_desirability, buf);
}
