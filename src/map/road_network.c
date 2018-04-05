#include "road_network.h"

#include "map/data.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

#include "Data/CityInfo.h"

#include <string.h>

#define MAX_QUEUE 1000

static const int ADJACENT_OFFSETS[] = {-162, 1, 162, -1};

static grid_u8 network;

static struct {
    int items[MAX_QUEUE];
    int head;
    int tail;
} queue;

void map_road_network_clear()
{
    map_grid_clear_u8(network.items);
}

int map_road_network_get(int grid_offset)
{
    return network.items[grid_offset];
}

static int mark_road_network(int grid_offset, uint8_t network_id)
{
    memset(&queue, 0, sizeof(queue));
    int guard = 0;
    int next_offset;
    int size = 1;
    do {
        if (++guard >= GRID_SIZE * GRID_SIZE) {
            break;
        }
        network.items[grid_offset] = network_id;
        next_offset = -1;
        for (int i = 0; i < 4; i++) {
            int new_offset = grid_offset + ADJACENT_OFFSETS[i];
            if (map_routing_citizen_is_passable(new_offset) && !network.items[new_offset]) {
                if (map_routing_citizen_is_road(new_offset) || map_terrain_is(new_offset, TERRAIN_ACCESS_RAMP)) {
                    network.items[new_offset] = network_id;
                    size++;
                    if (next_offset == -1) {
                        next_offset = new_offset;
                    } else {
                        queue.items[queue.tail++] = new_offset;
                        if (queue.tail >= MAX_QUEUE) {
                            queue.tail = 0;
                        }
                    }
                }
            }
        }
        if (next_offset == -1) {
            if (queue.head == queue.tail) {
                return size;
            }
            next_offset = queue.items[queue.head++];
            if (queue.head >= MAX_QUEUE) {
                queue.head = 0;
            }
        }
        grid_offset = next_offset;
    } while (next_offset > -1);
    return size;
}

static void clear_largest_networks()
{
    for (int i = 0; i < 10; i++) {
        Data_CityInfo.largestRoadNetworks[i].id = 0;
        Data_CityInfo.largestRoadNetworks[i].size = 0;
    }
}

static void record_in_largest_networks(int network_id, int size)
{
    for (int n = 0; n < 10; n++) {
        if (size > Data_CityInfo.largestRoadNetworks[n].size) {
            // move everyone down
            for (int m = 9; m > n; m--) {
                Data_CityInfo.largestRoadNetworks[m].id = Data_CityInfo.largestRoadNetworks[m-1].id;
                Data_CityInfo.largestRoadNetworks[m].size = Data_CityInfo.largestRoadNetworks[m-1].size;
            }
            Data_CityInfo.largestRoadNetworks[n].id = network_id;
            Data_CityInfo.largestRoadNetworks[n].size = size;
            break;
        }
    }
}

void map_road_network_update()
{
    clear_largest_networks();
    map_grid_clear_u8(network.items);
    int network_id = 1;
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_terrain_is(grid_offset, TERRAIN_ROAD) && !network.items[grid_offset]) {
                int size = mark_road_network(grid_offset, network_id);
                record_in_largest_networks(network_id, size);
                network_id++;
            }
        }
    }
}
