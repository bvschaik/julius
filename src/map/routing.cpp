#include "routing.h"

#include "map/grid.h"
#include "map/road_aqueduct.h"

#include "Data/Building.h"
#include "Data/Figure.h"
#include "Data/Grid.h"
#include "Data/Routes.h"
#include "../Routing.h"

#define MAX_QUEUE GRID_SIZE * GRID_SIZE
#define GUARD 50000

static const int ROUTE_OFFSETS[] = {-162, 1, 162, -1, -161, 163, 161, -163};

static struct {
	int head;
	int tail;
	int items[MAX_QUEUE];
} queue;

static struct {
    uint8_t water_drag[GRID_SIZE * GRID_SIZE];
} grid;

static struct {
    int throughBuildingId;
    int isAqueduct;
} state;

static void enqueue(int next_offset, int dist)
{
    Data_Grid_routingDistance[next_offset] = dist;
    queue.items[queue.tail++] = next_offset;
    if (queue.tail >= MAX_QUEUE) {
        queue.tail = 0;
    }
}

static int valid_offset(int grid_offset)
{
    return grid_offset >= 0 && grid_offset < GRID_SIZE * GRID_SIZE &&
        Data_Grid_routingDistance[grid_offset] == 0;
}

static void route_queue(int source, int dest, void (*callback)(int next_offset, int dist))
{
    map_grid_clear_u16(Data_Grid_routingDistance);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (offset == dest) {
            break;
        }
        int dist = 1 + Data_Grid_routingDistance[offset];
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
    map_grid_clear_u16(Data_Grid_routingDistance);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        int dist = 1 + Data_Grid_routingDistance[offset];
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
    map_grid_clear_u16(Data_Grid_routingDistance);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (offset == dest) break;
        if (++tiles > max_tiles) break;
        int dist = 1 + Data_Grid_routingDistance[offset];
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
    map_grid_clear_u16(Data_Grid_routingDistance);
    map_grid_clear_u8(grid.water_drag);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        int offset = queue.items[queue.head];
        if (++tiles > GUARD) {
            break;
        }
        int drag = Data_Grid_routingWater[offset] == Routing_Water_m2_MapEdge ? 4 : 0;
        if (drag && grid.water_drag[offset]++ < drag) {
            queue.items[queue.tail++] = offset;
            if (queue.tail >= MAX_QUEUE) {
                queue.tail = 0;
            }
        } else {
            int dist = 1 + Data_Grid_routingDistance[offset];
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
    map_grid_clear_u16(Data_Grid_routingDistance);
    queue.head = queue.tail = 0;
    enqueue(source, 1);
    int tiles = 0;
    while (queue.head != queue.tail) {
        if (++tiles > GUARD) {
            break;
        }
        int offset = queue.items[queue.head];
        int dist = 1 + Data_Grid_routingDistance[offset];
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
    if (Data_Grid_routingLandCitizen[next_offset] >= Routing_Citizen_0_Road) {
        enqueue(next_offset, dist);
    }
}

void map_routing_calculate_distances(int x, int y)
{
    ++Data_Routes.totalRoutesCalculated;
    route_queue(map_grid_offset(x, y), -1, callback_calc_distance);
}

static void callback_calc_distance_water_boat(int next_offset, int dist)
{
    if (Data_Grid_routingWater[next_offset] != Routing_Water_m1_Blocked &&
        Data_Grid_routingWater[next_offset] != Routing_Water_m3_LowBridge) {
        enqueue(next_offset, dist);
        if (Data_Grid_routingWater[next_offset] == Routing_Water_m2_MapEdge) {
            Data_Grid_routingDistance[next_offset] += 4;
        }
    }
}

void map_routing_calculate_distances_water_boat(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (Data_Grid_routingWater[grid_offset] == Routing_Water_m1_Blocked) {
        map_grid_clear_u16(Data_Grid_routingDistance);
    } else {
        route_queue_boat(grid_offset, callback_calc_distance_water_boat);
    }
}

static void callback_calc_distance_water_flotsam(int next_offset, int dist)
{
    if (Data_Grid_routingWater[next_offset] != Routing_Water_m1_Blocked) {
        enqueue(next_offset, dist);
    }
}

void map_routing_calculate_distances_water_flotsam(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    if (Data_Grid_routingWater[grid_offset] == Routing_Water_m1_Blocked) {
        map_grid_clear_u16(Data_Grid_routingDistance);
    } else {
        route_queue_dir8(grid_offset, callback_calc_distance_water_flotsam);
    }
}

static void callback_calc_distance_build_wall(int next_offset, int dist)
{
    if (Data_Grid_routingLandCitizen[next_offset] == Routing_Citizen_4_ClearTerrain) {
        enqueue(next_offset, dist);
    }
}

static void callback_calc_distance_build_road(int next_offset, int dist)
{
    int blocked = 0;
    switch (Data_Grid_routingLandCitizen[next_offset]) {
        case Routing_Citizen_m3_Aqueduct:
            if (!map_can_place_road_under_aqueduct(next_offset)) {
                Data_Grid_routingDistance[next_offset] = -1;
                blocked = 1;
            }
            break;
        case Routing_Citizen_2_PassableTerrain: // rubble, garden, access ramp
        case Routing_Citizen_m1_Blocked: // non-empty land
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
    switch (Data_Grid_routingLandCitizen[next_offset]) {
        case Routing_Citizen_m3_Aqueduct:
        case Routing_Citizen_2_PassableTerrain: // rubble, garden, access ramp
        case Routing_Citizen_m1_Blocked: // non-empty land
            blocked = 1;
            break;
        default:
            if (Data_Grid_terrain[next_offset] & Terrain_Building) {
                if (Data_Grid_routingLandCitizen[next_offset] != Routing_Citizen_m4_ReservoirConnector) {
                    blocked = 1;
                }
            }
            break;
    }
    if (Data_Grid_terrain[next_offset] & Terrain_Road && !map_can_place_aqueduct_on_road(next_offset)) {
        Data_Grid_routingDistance[next_offset] = -1;
        blocked = 1;
    }
    if (!blocked) {
        enqueue(next_offset, dist);
    }
}

static int map_can_place_initial_road_or_aqueduct(int gridOffset, int isAqueduct)
{
    if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_m1_Blocked) {
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
            if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_RESERVOIR) {
                return 1;
            }
        }
        return 0;
    } else if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_2_PassableTerrain) {
        // rubble, access ramp, garden
        return 0;
    } else if (Data_Grid_routingLandCitizen[gridOffset] == Routing_Citizen_m3_Aqueduct) {
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
    ++Data_Routes.totalRoutesCalculated;
    if (type == ROUTED_BUILDING_ROAD) {
        route_queue(source_offset, -1, callback_calc_distance_build_road);
    } else {
        route_queue(source_offset, -1, callback_calc_distance_build_aqueduct);
    }
    return 1;
}

static int callback_delete_wall_aqueduct(int next_offset, int dist)
{
    if (Data_Grid_routingLandCitizen[next_offset] < Routing_Citizen_0_Road) {
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
    ++Data_Routes.totalRoutesCalculated;
    route_queue_until(map_grid_offset(x, y), callback_delete_wall_aqueduct);
}

static int has_fighting_friendly(int grid_offset)
{
    int figure_id = Data_Grid_figureIds[grid_offset];
    if (figure_id > 0) {
        while (figure_id) {
            if (Data_Figures[figure_id].isFriendly &&
                Data_Figures[figure_id].actionState == FigureActionState_150_Attack) {
                return 1;
            }
            figure_id = Data_Figures[figure_id].nextFigureIdOnSameTile;
        }
    }
    return 0;
}

static int has_fighting_enemy(int grid_offset)
{
    int figure_id = Data_Grid_figureIds[grid_offset];
    if (figure_id > 0) {
        while (figure_id) {
            if (!Data_Figures[figure_id].isFriendly &&
                Data_Figures[figure_id].actionState == FigureActionState_150_Attack) {
                return 1;
            }
            figure_id = Data_Figures[figure_id].nextFigureIdOnSameTile;
        }
    }
    return 0;
}

static void callback_travel_citizen_land(int next_offset, int dist)
{
    if (Data_Grid_routingLandCitizen[next_offset] >= 0 && !has_fighting_friendly(next_offset)) {
        enqueue(next_offset, dist);
    }
}

int map_routing_citizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++Data_Routes.totalRoutesCalculated;
    route_queue(sourceOffset, destOffset, callback_travel_citizen_land);
    return Data_Grid_routingDistance[destOffset] != 0;
}

static void callback_travel_citizen_road_garden(int next_offset, int dist)
{
    if (Data_Grid_routingLandCitizen[next_offset] >= Routing_Citizen_0_Road &&
        Data_Grid_routingLandCitizen[next_offset] <= Routing_Citizen_2_PassableTerrain) {
        enqueue(next_offset, dist);
    }
}

int map_routing_citizen_can_travel_over_road_garden(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++Data_Routes.totalRoutesCalculated;
    route_queue(sourceOffset, destOffset, callback_travel_citizen_road_garden);
    return Data_Grid_routingDistance[destOffset] != 0;
}

static void callback_travel_walls(int next_offset, int dist)
{
    if (Data_Grid_routingWalls[next_offset] >= Routing_Wall_0_Passable &&
        Data_Grid_routingWalls[next_offset] <= 2) {
        enqueue(next_offset, dist);
    }
}

int map_routing_can_travel_over_walls(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++Data_Routes.totalRoutesCalculated;
    route_queue(sourceOffset, destOffset, callback_travel_walls);
    return Data_Grid_routingDistance[destOffset] != 0;
}

static void callback_travel_noncitizen_land_through_building(int next_offset, int dist)
{
    if (!has_fighting_enemy(next_offset)) {
        if (Data_Grid_routingLandNonCitizen[next_offset] == Routing_NonCitizen_0_Passable ||
            Data_Grid_routingLandNonCitizen[next_offset] == Routing_NonCitizen_2_Clearable ||
            (Data_Grid_routingLandNonCitizen[next_offset] == Routing_NonCitizen_1_Building &&
                Data_Grid_buildingIds[next_offset] == state.throughBuildingId)) {
            enqueue(next_offset, dist);
        }
    }
}

static void callback_travel_noncitizen_land(int next_offset, int dist)
{
    if (!has_fighting_enemy(next_offset)) {
        if (Data_Grid_routingLandNonCitizen[next_offset] >= Routing_NonCitizen_0_Passable &&
            Data_Grid_routingLandNonCitizen[next_offset] < Routing_NonCitizen_5_Fort) {
            enqueue(next_offset, dist);
        }
    }
}

int map_routing_noncitizen_can_travel_over_land(int src_x, int src_y, int dst_x, int dst_y, int onlyThroughBuildingId, int maxTiles)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++Data_Routes.totalRoutesCalculated;
    ++Data_Routes.enemyRoutesCalculated;
    if (onlyThroughBuildingId) {
        state.throughBuildingId = onlyThroughBuildingId;
        route_queue(sourceOffset, destOffset, callback_travel_noncitizen_land_through_building);
    } else {
        route_queue_max(sourceOffset, destOffset, maxTiles, callback_travel_noncitizen_land);
    }
    return Data_Grid_routingDistance[destOffset] != 0;
}

static void callback_travel_noncitizen_through_everything(int next_offset, int dist)
{
    if (Data_Grid_routingLandNonCitizen[next_offset] >= Routing_NonCitizen_0_Passable) {
        enqueue(next_offset, dist);
    }
}

int map_routing_noncitizen_can_travel_through_everything(int src_x, int src_y, int dst_x, int dst_y)
{
    int sourceOffset = map_grid_offset(src_x, src_y);
    int destOffset = map_grid_offset(dst_x, dst_y);
    ++Data_Routes.totalRoutesCalculated;
    route_queue(sourceOffset, destOffset, callback_travel_noncitizen_through_everything);
    return Data_Grid_routingDistance[destOffset] != 0;
}


int map_routing_distance(int grid_offset)
{
    return Data_Grid_routingDistance[grid_offset];
}

int map_routing_is_wall_passable(int grid_offset)
{
    return Data_Grid_routingWalls[grid_offset] == 0;
}

int map_routing_citizen_is_passable(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road ||
        Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_citizen_is_road(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road;
}

int map_routing_citizen_is_passable_terrain(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_noncitizen_terrain(int grid_offset)
{
    return Data_Grid_routingLandNonCitizen[grid_offset];
}

void map_routing_save_state(buffer *buf)
{
    buffer_write_i32(buf, 0); // unused counter
    buffer_write_i32(buf, Data_Routes.enemyRoutesCalculated);
    buffer_write_i32(buf, Data_Routes.totalRoutesCalculated);
    buffer_write_i32(buf, 0); // unused counter
}

void map_routing_load_state(buffer *buf)
{
    buffer_skip(buf, 4); // unused counter
    Data_Routes.enemyRoutesCalculated = buffer_read_i32(buf);
    Data_Routes.totalRoutesCalculated = buffer_read_i32(buf);
    buffer_skip(buf, 4); // unused counter
}
