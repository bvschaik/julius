#include "routing.h"

#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/road_aqueduct.h"
#include "map/routing_data.h"

#include "Data/Building.h"
#include "Data/State.h"

#define MAX_QUEUE GRID_SIZE * GRID_SIZE
#define GUARD 50000

static const int ROUTE_OFFSETS[] = {-162, 1, 162, -1, -161, 163, 161, -163};

static grid_u16 routing_distance;

static struct {
    int total_routes_calculated;
    int enemy_routes_calculated;
} stats = {0, 0};

static struct {
	int head;
	int tail;
	int items[MAX_QUEUE];
} queue;

static grid_u8 water_drag;

static struct {
    int throughBuildingId;
} state;

static void enqueue(int next_offset, int dist)
{
    routing_distance.items[next_offset] = dist;
    queue.items[queue.tail++] = next_offset;
    if (queue.tail >= MAX_QUEUE) {
        queue.tail = 0;
    }
}

static int valid_offset(int grid_offset)
{
    return grid_offset >= 0 && grid_offset < GRID_SIZE * GRID_SIZE &&
        routing_distance.items[grid_offset] == 0;
}

static void route_queue(int source, int dest, void (*callback)(int next_offset, int dist))
{
    map_grid_clear_u16(routing_distance.items);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (offset == dest) {
            break;
        }
        int dist = 1 + routing_distance.items[offset];
        for (int i = 0; i < 4; i++) {
            if (valid_offset(offset + ROUTE_OFFSETS[i])) {
                callback(offset + ROUTE_OFFSETS[i], dist);
            }
        }
        if (++queue.head >= MAX_QUEUE) {
            queue.head = 0;
        }
    }
}

static void route_queue_until(int source, int (*callback)(int next_offset, int dist))
{
    map_grid_clear_u16(routing_distance.items);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        int dist = 1 + routing_distance.items[offset];
        for (int i = 0; i < 4; i++) {
            if (valid_offset(offset + ROUTE_OFFSETS[i])) {
                if (!callback(offset + ROUTE_OFFSETS[i], dist)) {
                    break;
                }
            }
        }
        if (++queue.head >= MAX_QUEUE) {
            queue.head = 0;
        }
    }
}

static void route_queue_max(int source, int dest, int max_tiles, void (*callback)(int, int))
{
    map_grid_clear_u16(routing_distance.items);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (offset == dest) break;
        if (++tiles > max_tiles) break;
        int dist = 1 + routing_distance.items[offset];
        for (int i = 0; i < 4; i++) {
            if (valid_offset(offset + ROUTE_OFFSETS[i])) {
                callback(offset + ROUTE_OFFSETS[i], dist);
            }
        }
        if (++queue.head >= MAX_QUEUE) {
            queue.head = 0;
        }
    }
}

static void route_queue_boat(int source, void (*callback)(int, int))
{
    map_grid_clear_u16(routing_distance.items);
    map_grid_clear_u8(water_drag.items);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (++tiles > GUARD) {
            break;
        }
        int drag = terrain_water.items[offset] == WATER_N2_MAP_EDGE ? 4 : 0;
        if (drag && water_drag.items[offset]++ < drag) {
            queue.items[queue.tail++] = offset;
            if (queue.tail >= MAX_QUEUE) {
                queue.tail = 0;
            }
        } else {
            int dist = 1 + routing_distance.items[offset];
            for (int i = 0; i < 4; i++) {
                if (valid_offset(offset + ROUTE_OFFSETS[i])) {
                    callback(offset + ROUTE_OFFSETS[i], dist);
                }
            }
        }
        if (++queue.head >= MAX_QUEUE) {
            queue.head = 0;
        }
    }
}

static void route_queue_dir8(int source, void (*callback)(int, int))
{
    map_grid_clear_u16(routing_distance.items);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        if (++tiles > GUARD) {
            break;
        }
        int offset = queue.items[queue.head];
        int dist = 1 + routing_distance.items[offset];
        for (int i = 0; i < 8; i++) {
            if (valid_offset(offset + ROUTE_OFFSETS[i])) {
                callback(offset + ROUTE_OFFSETS[i], dist);
            }
        }
        if (++queue.head >= MAX_QUEUE) {
            queue.head = 0;
        }
    }
}

static void callback_calc_distance(int next_offset, int dist)
{
    if (terrain_land_citizen.items[next_offset] >= CITIZEN_0_ROAD) {
        enqueue(next_offset, dist);
    }
}

void map_routing_calculate_distances(int x, int y)
{
    ++stats.total_routes_calculated;
    route_queue(map_grid_offset(x, y), -1, callback_calc_distance);
}

static void callback_calc_distance_water_boat(int next_offset, int dist)
{
    if (terrain_water.items[next_offset] != WATER_N1_BLOCKED &&
        terrain_water.items[next_offset] != WATER_N3_LOW_BRIDGE) {
        enqueue(next_offset, dist);
        if (terrain_water.items[next_offset] == WATER_N2_MAP_EDGE) {
            routing_distance.items[next_offset] += 4;
        }
    }
}

void map_routing_calculate_distances_water_boat(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
    } else {
        route_queue_boat(grid_offset, callback_calc_distance_water_boat);
    }
}

static void callback_calc_distance_water_flotsam(int next_offset, int dist)
{
    if (terrain_water.items[next_offset] != WATER_N1_BLOCKED) {
        enqueue(next_offset, dist);
    }
}

void map_routing_calculate_distances_water_flotsam(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
        map_grid_clear_u16(routing_distance.items);
    } else {
        route_queue_dir8(grid_offset, callback_calc_distance_water_flotsam);
    }
}

static void callback_calc_distance_build_wall(int next_offset, int dist)
{
    if (terrain_land_citizen.items[next_offset] == CITIZEN_4_CLEAR_TERRAIN) {
        enqueue(next_offset, dist);
    }
}

static void callback_calc_distance_build_road(int next_offset, int dist)
{
    int blocked = 0;
    switch (terrain_land_citizen.items[next_offset]) {
        case CITIZEN_N3_AQUEDUCT:
            if (!map_can_place_road_under_aqueduct(next_offset)) {
                routing_distance.items[next_offset] = -1;
                blocked = 1;
            }
            break;
        case CITIZEN_2_PASSABLE_TERRAIN: // rubble, garden, access ramp
        case CITIZEN_N1_BLOCKED: // non-empty land
            blocked = 1;
            break;
        default:
            if (Data_Grid_terrain[next_offset] & Terrain_Building) {
                blocked = 1;
            }
            break;
    }
    if (!blocked) {
        enqueue(next_offset, dist);
    }
}

static void callback_calc_distance_build_aqueduct(int next_offset, int dist)
{
    int blocked = 0;
    switch (terrain_land_citizen.items[next_offset]) {
        case CITIZEN_N3_AQUEDUCT:
        case CITIZEN_2_PASSABLE_TERRAIN: // rubble, garden, access ramp
        case CITIZEN_N1_BLOCKED: // non-empty land
            blocked = 1;
            break;
        default:
            if (Data_Grid_terrain[next_offset] & Terrain_Building) {
                if (terrain_land_citizen.items[next_offset] != CITIZEN_N4_RESERVOIR_CONNECTOR) {
                    blocked = 1;
                }
            }
            break;
    }
    if (Data_Grid_terrain[next_offset] & Terrain_Road && !map_can_place_aqueduct_on_road(next_offset)) {
        routing_distance.items[next_offset] = -1;
        blocked = 1;
    }
    if (!blocked) {
        enqueue(next_offset, dist);
    }
}

static int map_can_place_initial_road_or_aqueduct(int gridOffset, int isAqueduct)
{
    if (terrain_land_citizen.items[gridOffset] == CITIZEN_N1_BLOCKED) {
        // not open land, can only if:
        // - aqueduct should be placed, and:
        // - land is a reservoir building OR an aqueduct
        if (!isAqueduct) {
            return 0;
        }
        if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
            return 1;
        }
        if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
            if (Data_Buildings[map_building_at(gridOffset)].type == BUILDING_RESERVOIR) {
                return 1;
            }
        }
        return 0;
    } else if (terrain_land_citizen.items[gridOffset] == CITIZEN_2_PASSABLE_TERRAIN) {
        // rubble, access ramp, garden
        return 0;
    } else if (terrain_land_citizen.items[gridOffset] == CITIZEN_N3_AQUEDUCT) {
        if (isAqueduct) {
            return 0;
        }
        if (map_can_place_road_under_aqueduct(gridOffset)) {
            return 1;
        }
        return 0;
    } else {
        return 1;
    }
}

int map_routing_calculate_distances_for_building(routed_building_type type, int x, int y)
{
    if (type == ROUTED_BUILDING_WALL) {
        route_queue(map_grid_offset(x, y), -1, callback_calc_distance_build_wall);
        return 1;
    }
    int source_offset = map_grid_offset(x, y);
    if (!map_can_place_initial_road_or_aqueduct(source_offset, type != ROUTED_BUILDING_ROAD)) {
        return 0;
    }
    if (Data_Grid_terrain[source_offset] & Terrain_Road &&
        type != ROUTED_BUILDING_ROAD && !map_can_place_aqueduct_on_road(source_offset)) {
        return 0;
    }
    ++stats.total_routes_calculated;
    if (type == ROUTED_BUILDING_ROAD) {
        route_queue(source_offset, -1, callback_calc_distance_build_road);
    } else {
        route_queue(source_offset, -1, callback_calc_distance_build_aqueduct);
    }
    return 1;
}

static int callback_delete_wall_aqueduct(int next_offset, int dist)
{
    if (terrain_land_citizen.items[next_offset] < CITIZEN_0_ROAD) {
        if (Data_Grid_terrain[next_offset] & (Terrain_Aqueduct | Terrain_Wall)) {
            Data_Grid_terrain[next_offset] &= Terrain_2e80;
            return 1;
        }
    } else {
        enqueue(next_offset, dist);
    }
    return 0;
}

void map_routing_delete_first_wall_or_aqueduct(int x, int y)
{
    ++stats.total_routes_calculated;
    route_queue_until(map_grid_offset(x, y), callback_delete_wall_aqueduct);
}

static int is_fighting_friendly(figure *f)
{
    return f->isFriendly && f->actionState == FigureActionState_150_Attack;
}

static int has_fighting_friendly(int grid_offset)
{
    return map_figure_foreach_until(grid_offset, is_fighting_friendly);
}

static int is_fighting_enemy(figure *f)
{
    return !f->isFriendly && f->actionState == FigureActionState_150_Attack;
}

static int has_fighting_enemy(int grid_offset)
{
    return map_figure_foreach_until(grid_offset, is_fighting_enemy);
}

static void callback_travel_citizen_land(int next_offset, int dist)
{
    if (terrain_land_citizen.items[next_offset] >= 0 && !has_fighting_friendly(next_offset)) {
        enqueue(next_offset, dist);
    }
}

int map_routing_citizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++stats.total_routes_calculated;
    route_queue(sourceOffset, destOffset, callback_travel_citizen_land);
    return routing_distance.items[destOffset] != 0;
}

static void callback_travel_citizen_road_garden(int next_offset, int dist)
{
    if (terrain_land_citizen.items[next_offset] >= CITIZEN_0_ROAD &&
        terrain_land_citizen.items[next_offset] <= CITIZEN_2_PASSABLE_TERRAIN) {
        enqueue(next_offset, dist);
    }
}

int map_routing_citizen_can_travel_over_road_garden(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++stats.total_routes_calculated;
    route_queue(sourceOffset, destOffset, callback_travel_citizen_road_garden);
    return routing_distance.items[destOffset] != 0;
}

static void callback_travel_walls(int next_offset, int dist)
{
    if (terrain_walls.items[next_offset] >= WALL_0_PASSABLE &&
        terrain_walls.items[next_offset] <= 2) {
        enqueue(next_offset, dist);
    }
}

int map_routing_can_travel_over_walls(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++stats.total_routes_calculated;
    route_queue(sourceOffset, destOffset, callback_travel_walls);
    return routing_distance.items[destOffset] != 0;
}

static void callback_travel_noncitizen_land_through_building(int next_offset, int dist)
{
    if (!has_fighting_enemy(next_offset)) {
        if (terrain_land_noncitizen.items[next_offset] == NONCITIZEN_0_PASSABLE ||
            terrain_land_noncitizen.items[next_offset] == NONCITIZEN_2_CLEARABLE ||
            (terrain_land_noncitizen.items[next_offset] == NONCITIZEN_1_BUILDING &&
                map_building_at(next_offset) == state.throughBuildingId)) {
            enqueue(next_offset, dist);
        }
    }
}

static void callback_travel_noncitizen_land(int next_offset, int dist)
{
    if (!has_fighting_enemy(next_offset)) {
        if (terrain_land_noncitizen.items[next_offset] >= NONCITIZEN_0_PASSABLE &&
            terrain_land_noncitizen.items[next_offset] < NONCITIZEN_5_FORT) {
            enqueue(next_offset, dist);
        }
    }
}

int map_routing_noncitizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y, int onlyThroughBuildingId, int maxTiles)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++stats.total_routes_calculated;
    ++stats.enemy_routes_calculated;
    if (onlyThroughBuildingId) {
        state.throughBuildingId = onlyThroughBuildingId;
        route_queue(sourceOffset, destOffset, callback_travel_noncitizen_land_through_building);
    } else {
        route_queue_max(sourceOffset, destOffset, maxTiles, callback_travel_noncitizen_land);
    }
    return routing_distance.items[destOffset] != 0;
}

static void callback_travel_noncitizen_through_everything(int next_offset, int dist)
{
    if (terrain_land_noncitizen.items[next_offset] >= NONCITIZEN_0_PASSABLE) {
        enqueue(next_offset, dist);
    }
}

int map_routing_noncitizen_can_travel_through_everything(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++stats.total_routes_calculated;
    route_queue(sourceOffset, destOffset, callback_travel_noncitizen_through_everything);
    return routing_distance.items[destOffset] != 0;
}

void map_routing_block(int x, int y, int size)
{
    if (IsOutsideMap(x, y, size)) {
        return;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            routing_distance.items[map_grid_offset(x+dx, y+dy)] = 0;
        }
    }
}

int map_routing_distance(int grid_offset)
{
    return routing_distance.items[grid_offset];
}

void map_routing_save_state(buffer *buf)
{
    buffer_write_i32(buf, 0); // unused counter
    buffer_write_i32(buf, stats.enemy_routes_calculated);
    buffer_write_i32(buf, stats.total_routes_calculated);
    buffer_write_i32(buf, 0); // unused counter
}

void map_routing_load_state(buffer *buf)
{
    buffer_skip(buf, 4); // unused counter
    stats.enemy_routes_calculated = buffer_read_i32(buf);
    stats.total_routes_calculated = buffer_read_i32(buf);
    buffer_skip(buf, 4); // unused counter
}
