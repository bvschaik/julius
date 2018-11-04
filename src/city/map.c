#include "map.h"

#include "city/data_private.h"
#include "map/grid.h"

const map_tile *city_map_entry_point(void)
{
    return &city_data.map.entry_point;
}

const map_tile *city_map_exit_point(void)
{
    return &city_data.map.exit_point;
}

const map_tile *city_map_entry_flag(void)
{
    return &city_data.map.entry_flag;
}

const map_tile *city_map_exit_flag(void)
{
    return &city_data.map.exit_flag;
}

static int set_tile(map_tile *tile, int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    tile->x = x;
    tile->y = y;
    tile->grid_offset = grid_offset;
    return grid_offset;
}

void city_map_set_entry_point(int x, int y)
{
    set_tile(&city_data.map.entry_point, x, y);
}

void city_map_set_exit_point(int x, int y)
{
    set_tile(&city_data.map.exit_point, x, y);
}

int city_map_set_entry_flag(int x, int y)
{
    return set_tile(&city_data.map.entry_flag, x, y);
}

int city_map_set_exit_flag(int x, int y)
{
    return set_tile(&city_data.map.exit_flag, x, y);
}

int city_map_road_network_index(int network_id)
{
    for (int n = 0; n < 10; n++) {
        if (city_data.map.largest_road_networks[n].id == network_id) {
            return n;
        }
    }
    return 11;
}

void city_map_clear_largest_road_networks(void)
{
    for (int i = 0; i < 10; i++) {
        city_data.map.largest_road_networks[i].id = 0;
        city_data.map.largest_road_networks[i].size = 0;
    }
}

void city_map_add_to_largest_road_networks(int network_id, int size)
{
    for (int n = 0; n < 10; n++) {
        if (size > city_data.map.largest_road_networks[n].size) {
            // move everyone down
            for (int m = 9; m > n; m--) {
                city_data.map.largest_road_networks[m].id = city_data.map.largest_road_networks[m-1].id;
                city_data.map.largest_road_networks[m].size = city_data.map.largest_road_networks[m-1].size;
            }
            city_data.map.largest_road_networks[n].id = network_id;
            city_data.map.largest_road_networks[n].size = size;
            break;
        }
    }
}
