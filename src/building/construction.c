#include "construction.h"

#include "Figure.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
#include "building/construction_building.h"
#include "building/construction_clear.h"
#include "building/construction_warning.h"
#include "building/count.h"
#include "building/model.h"
#include "building/properties.h"
#include "building/warehouse.h"
#include "city/finance.h"
#include "city/warning.h"
#include "core/calc.h"
#include "figure/formation.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/water.h"

struct reservoir_info {
    int cost;
    int place_reservoir_at_start;
    int place_reservoir_at_end;
};

enum {
    PlaceReservoir_Blocked = -1,
    PlaceReservoir_No = 0,
    PlaceReservoir_Yes = 1,
    PlaceReservoir_Exists = 2
};

static struct {
    building_type type;
    int in_progress;
    int x_start;
    int y_start;
    int x_end;
    int y_end;
    int cost;
} data;

static int last_items_cleared;

static int place_houses(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int needs_road_warning = 0;
    int items_placed = 0;
    game_undo_restore_building_state();
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x,y);
            if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                continue;
            }
            if (measure_only) {
                map_property_mark_constructing(grid_offset);
                items_placed++;
            } else {
                building *b = building_create(BUILDING_HOUSE_VACANT_LOT, x, y);
                game_undo_add_building(b);
                if (b->id > 0) {
                    items_placed++;
                    Terrain_addBuildingToGrids(b->id, x, y, 1,
                        image_group(GROUP_BUILDING_HOUSE_VACANT_LOT), TERRAIN_BUILDING);
                    if (!Terrain_existsTileWithinRadiusWithType(x, y, 1, 2, TERRAIN_ROAD)) {
                        needs_road_warning = 1;
                    }
                }
            }
        }
    }
    if (!measure_only) {
        building_construction_warning_check_food_stocks(BUILDING_HOUSE_VACANT_LOT);
        if (needs_road_warning) {
            city_warning_show(WARNING_HOUSE_TOO_FAR_FROM_ROAD);
        }
        map_routing_update_land();
        UI_Window_requestRefresh();
    }
    return items_placed;
}

static int place_routed_building(int x_start, int y_start, int x_end, int y_end, routed_building_type type, int *items)
{
    static const int direction_indices[8][4] = {
        {0, 2, 6, 4},
        {0, 2, 6, 4},
        {2, 4, 0, 6},
        {2, 4, 0, 6},
        {4, 6, 2, 0},
        {4, 6, 2, 0},
        {6, 0, 4, 2},
        {6, 0, 4, 2}
    };
    *items = 0;
    int grid_offset = map_grid_offset(x_end, y_end);
    int guard = 0;
    // reverse routing
    while (1) {
        if (++guard >= 400) {
            return 0;
        }
        int distance = map_routing_distance(grid_offset);
        if (distance <= 0) {
            return 0;
        }
        switch (type) {
            default:
            case ROUTED_BUILDING_ROAD:
                *items += TerrainGraphics_setTileRoad(x_end, y_end);
                break;
            case ROUTED_BUILDING_WALL:
                *items += TerrainGraphics_setTileWall(x_end, y_end);
                break;
            case ROUTED_BUILDING_AQUEDUCT:
                *items += TerrainGraphics_setTileAqueductTerrain(x_end, y_end);
                break;
            case ROUTED_BUILDING_AQUEDUCT_WITHOUT_GRAPHIC:
                *items += 1;
                break;
        }
        int direction = calc_general_direction(x_end, y_end, x_start, y_start);
        if (direction == DIR_8_NONE) {
            return 1; // destination reached
        }
        int routed = 0;
        for (int i = 0; i < 4; i++) {
            int index = direction_indices[direction][i];
            int new_grid_offset = grid_offset + map_grid_direction_delta(index);
            int new_dist = map_routing_distance(new_grid_offset);
            if (new_dist > 0 && new_dist < distance) {
                grid_offset = new_grid_offset;
                x_end = map_grid_offset_to_x(grid_offset);
                y_end = map_grid_offset_to_y(grid_offset);
                routed = 1;
                break;
            }
        }
        if (!routed) {
            return 0;
        }
    }
}

static int place_road(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    game_undo_restore_map(0);

    int start_offset = map_grid_offset(x_start, y_start);
    int end_offset = map_grid_offset(x_end, y_end);
    int forbidden_terrain_mask =
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ELEVATION |
        TERRAIN_RUBBLE | TERRAIN_BUILDING | TERRAIN_WALL;
    if (map_terrain_is(start_offset, forbidden_terrain_mask)) {
        return 0;
    }
    if (map_terrain_is(end_offset, forbidden_terrain_mask)) {
        return 0;
    }
    
    int items_placed = 0;
    if (map_routing_calculate_distances_for_building(ROUTED_BUILDING_ROAD, x_start, y_start) &&
            place_routed_building(x_start, y_start, x_end, y_end, ROUTED_BUILDING_ROAD, &items_placed)) {
        if (!measure_only) {
            map_routing_update_land();
            UI_Window_requestRefresh();
        }
    }
    return items_placed;
}

static int place_wall(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    game_undo_restore_map(0);

    int start_offset = map_grid_offset(x_start, y_start);
    int end_offset = map_grid_offset(x_end, y_end);
    int forbidden_terrain_mask =
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SCRUB |
        TERRAIN_ROAD | TERRAIN_GARDEN | TERRAIN_ELEVATION |
        TERRAIN_RUBBLE | TERRAIN_AQUEDUCT | TERRAIN_ACCESS_RAMP;
    if (map_terrain_is(start_offset, forbidden_terrain_mask)) {
        return 0;
    }
    if (map_terrain_is(end_offset, forbidden_terrain_mask)) {
        return 0;
    }
    int items_placed = 0;
    if (place_routed_building(x_start, y_start, x_end, y_end, ROUTED_BUILDING_WALL, &items_placed)) {
        if (!measure_only) {
            map_routing_update_land();
            map_routing_update_walls();
            UI_Window_requestRefresh();
        }
    }
    return items_placed;
}

static int place_plaza(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);
    game_undo_restore_map(1);
    
    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_terrain_is(grid_offset, TERRAIN_ROAD) &&
                !map_terrain_is(grid_offset, TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_AQUEDUCT)) {
                if (!map_property_is_plaza_or_earthquake(grid_offset)) {
                    items_placed++;
                }
                map_image_set(grid_offset, 0);
                map_property_mark_plaza_or_earthquake(grid_offset);
                map_property_set_multi_tile_size(grid_offset, 1);
                map_property_mark_draw_tile(grid_offset);
            }
        }
    }
    TerrainGraphics_updateRegionPlazas(0, 0,
        Data_State.map.width - 1, Data_State.map.height - 1);
    return items_placed;
}

static int place_garden(int x_start, int y_start, int x_end, int y_end)
{
    game_undo_restore_map(1);

    int x_min, y_min, x_max, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int items_placed = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x,y);
            if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                items_placed++;
                map_terrain_add(grid_offset, TERRAIN_GARDEN);
            }
        }
    }
    TerrainGraphics_updateAllGardens();
    return items_placed;
}

static int place_aqueduct(int measure_only, int x_start, int y_start, int x_end, int y_end, int *cost)
{
    game_undo_restore_map(0);

    int item_cost = model_get_building(BUILDING_AQUEDUCT)->cost;
    *cost = 0;
    int blocked = 0;
    int grid_offset = map_grid_offset(x_start, y_start);
    if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        if (map_property_is_plaza_or_earthquake(grid_offset)) {
            blocked = 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        blocked = 1;
    }
    grid_offset = map_grid_offset(x_end, y_end);
    if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
        if (map_property_is_plaza_or_earthquake(grid_offset)) {
            blocked = 1;
        }
    } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        blocked = 1;
    }
    if (blocked) {
        return 0;
    }
    if (!map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, x_start, y_start)) {
        return 0;
    }
    int num_items;
    place_routed_building(x_start, y_start, x_end, y_end, ROUTED_BUILDING_AQUEDUCT, &num_items);
    *cost = item_cost * num_items;
    return 1;
}

static int place_reservoir_and_aqueducts(int measure_only, int x_start, int y_start, int x_end, int y_end, struct reservoir_info *info)
{
    info->cost = 0;
    info->place_reservoir_at_start = PlaceReservoir_No;
    info->place_reservoir_at_end = PlaceReservoir_No;

    game_undo_restore_map(0);

    int distance = calc_maximum_distance(x_start, y_start, x_end, y_end);
    if (measure_only && !data.in_progress) {
        distance = 0;
    }
    if (distance > 0) {
        if (map_building_is_reservoir(x_start - 1, y_start - 1)) {
            info->place_reservoir_at_start = PlaceReservoir_Exists;
        } else if (Terrain_isClear(x_start - 1, y_start - 1, 3, TERRAIN_ALL, 0)) {
            info->place_reservoir_at_start = PlaceReservoir_Yes;
        } else {
            info->place_reservoir_at_start = PlaceReservoir_Blocked;
        }
    }
    if (map_building_is_reservoir(x_end - 1, y_end - 1)) {
        info->place_reservoir_at_end = PlaceReservoir_Exists;
    } else if (Terrain_isClear(x_end - 1, y_end - 1, 3, TERRAIN_ALL, 0)) {
        info->place_reservoir_at_end = PlaceReservoir_Yes;
    } else {
        info->place_reservoir_at_end = PlaceReservoir_Blocked;
    }
    if (info->place_reservoir_at_start == PlaceReservoir_Blocked || info->place_reservoir_at_end == PlaceReservoir_Blocked) {
        return 0;
    }
    if (info->place_reservoir_at_start == PlaceReservoir_Yes && info->place_reservoir_at_end == PlaceReservoir_Yes && distance < 3) {
        return 0;
    }
    if (!distance) {
        info->cost = model_get_building(BUILDING_RESERVOIR)->cost;
        return 1;
    }
    if (!map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT, x_start, y_start)) {
        return 0;
    }
    if (info->place_reservoir_at_start != PlaceReservoir_No) {
        map_routing_block(x_start - 1, y_start - 1, 3);
        Terrain_updateToPlaceBuildingToOverlay(3, x_start - 1, y_start - 1, TERRAIN_ALL, 1);
    }
    if (info->place_reservoir_at_end != PlaceReservoir_No) {
        map_routing_block(x_end - 1, y_end - 1, 3);
        Terrain_updateToPlaceBuildingToOverlay(3, x_end - 1, y_end - 1, TERRAIN_ALL, 1);
    }
    const int aqueduct_offsets_x[] = {0, 2, 0, -2};
    const int aqueduct_offsets_y[] = {-2, 0, 2, 0};
    int min_dist = 10000;
    int min_dir_start, min_dir_end;
    for (int dir_start = 0; dir_start < 4; dir_start++) {
        int dx_start = aqueduct_offsets_x[dir_start];
        int dy_start = aqueduct_offsets_y[dir_start];
        for (int dir_end = 0; dir_end < 4; dir_end++) {
            int dx_end = aqueduct_offsets_x[dir_end];
            int dy_end = aqueduct_offsets_y[dir_end];
            int dist;
            if (place_routed_building(
                    x_start + dx_start, y_start + dy_start, x_end + dx_end, y_end + dy_end,
                    ROUTED_BUILDING_AQUEDUCT_WITHOUT_GRAPHIC, &dist)) {
                if (dist && dist < min_dist) {
                    min_dist = dist;
                    min_dir_start = dir_start;
                    min_dir_end = dir_end;
                }
            }
        }
    }
    if (min_dist == 10000) {
        return 0;
    }
    int x_aq_start = aqueduct_offsets_x[min_dir_start];
    int y_aq_start = aqueduct_offsets_y[min_dir_start];
    int x_aq_end = aqueduct_offsets_x[min_dir_end];
    int y_aq_end = aqueduct_offsets_y[min_dir_end];
    int aq_items;
    place_routed_building(x_start + x_aq_start, y_start + y_aq_start,
        x_end + x_aq_end, y_end + y_aq_end, ROUTED_BUILDING_AQUEDUCT, &aq_items);
    if (info->place_reservoir_at_start == PlaceReservoir_Yes) {
        info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
    }
    if (info->place_reservoir_at_end == PlaceReservoir_Yes) {
        info->cost += model_get_building(BUILDING_RESERVOIR)->cost;
    }
    if (aq_items) {
        info->cost += aq_items * model_get_building(BUILDING_AQUEDUCT)->cost;
    }
    return 1;
}

void building_construction_reset(building_type type)
{
    data.type = type;
    data.in_progress = 0;
    data.x_start = 0;
    data.y_start = 0;
    data.x_end = 0;
    data.y_end = 0;
}

void building_construction_clear_type()
{
    data.cost = 0;
    data.type = BUILDING_NONE;
}

building_type building_construction_type()
{
    return data.type;
}

int building_construction_cost()
{
    return data.cost;
}

int building_construction_in_progress()
{
    return data.in_progress;
}

void building_construction_start(int x, int y)
{
    data.x_start = data.x_end = x;
    data.y_start = data.y_end = y;

    if (game_undo_start_build(data.type)) {
        data.in_progress = 1;
        switch (data.type) {
            case BUILDING_ROAD:
                map_routing_calculate_distances_for_building(ROUTED_BUILDING_ROAD,
                    data.x_start, data.y_start);
                break;
            case BUILDING_AQUEDUCT:
            case BUILDING_DRAGGABLE_RESERVOIR:
                map_routing_calculate_distances_for_building(ROUTED_BUILDING_AQUEDUCT,
                    data.x_start, data.y_start);
                break;
            case BUILDING_WALL:
                map_routing_calculate_distances_for_building(ROUTED_BUILDING_WALL,
                    data.x_start, data.y_start);
                break;
            default:
                break;
        }
    }
}

void building_construction_update(int x, int y)
{
    building_type type = data.type;
    data.x_end = x;
    data.y_end = y;
    if (!type || city_finance_out_of_money()) {
        data.cost = 0;
        return;
    }
    map_property_clear_constructing_and_deleted();
    int current_cost = model_get_building(type)->cost;

    if (type == BUILDING_CLEAR_LAND) {
        int items_placed = last_items_cleared = building_construction_clear_land(1, data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_WALL) {
        int items_placed = place_wall(1, data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_ROAD) {
        int items_placed = place_road(1, data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_PLAZA) {
        int items_placed = place_plaza(1, data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_GARDENS) {
        int items_placed = place_garden(data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
        int length = map_bridge_building_length();
        if (length > 1) current_cost *= length;
    } else if (type == BUILDING_AQUEDUCT) {
        place_aqueduct(1, data.x_start, data.y_start, x, y, &current_cost);
        TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
    } else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
        struct reservoir_info info;
        place_reservoir_and_aqueducts(1, data.x_start, data.y_start, x, y, &info);
        current_cost = info.cost;
        TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 1);
        Data_State.selectedBuilding.drawAsConstructing = 0;
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        int items_placed = place_houses(1, data.x_start, data.y_start, x, y);
        if (items_placed >= 0) current_cost *= items_placed;
    } else if (type == BUILDING_GATEHOUSE) {
        Terrain_updateToPlaceBuildingToOverlay(2, x, y, ~TERRAIN_ROAD, 0);
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        Terrain_updateToPlaceBuildingToOverlay(3, x, y, ~TERRAIN_ROAD, 0);
    } else if (type == BUILDING_WAREHOUSE) {
        Terrain_updateToPlaceBuildingToOverlay(3, x, y, TERRAIN_ALL, 0);
    } else if (type == BUILDING_FORT_LEGIONARIES || type == BUILDING_FORT_JAVELIN || type == BUILDING_FORT_MOUNTED) {
        if (formation_totals_get_num_legions() < 6) {
            const int offsets_x[] = {3, 4, 4, 3};
            const int offsets_y[] = {-1, -1, 0, 0};
            int orient_index = Data_State.map.orientation / 2;
            int x_offset = offsets_x[orient_index];
            int y_offset = offsets_y[orient_index];
            if (Terrain_isClearToBuild(3, x, y, TERRAIN_ALL) &&
                Terrain_isClearToBuild(4, x + x_offset, y + y_offset, TERRAIN_ALL)) {
                Terrain_updateToPlaceBuildingToOverlay(3, x, y, TERRAIN_ALL, 0);
            }
        }
    } else if (type == BUILDING_HIPPODROME) {
        if (Terrain_isClearToBuild(5, x, y, TERRAIN_ALL) &&
            Terrain_isClearToBuild(5, x + 5, y, TERRAIN_ALL) &&
            Terrain_isClearToBuild(5, x + 10, y, TERRAIN_ALL)) {
            Terrain_updateToPlaceBuildingToOverlay(5, x, y, TERRAIN_ALL, 0);
        }
    } else if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
        if (!map_water_determine_orientation_size2(x, y, 1, 0, 0)) {
            Data_State.selectedBuilding.drawAsConstructing = 1;
        }
    } else if (type == BUILDING_DOCK) {
        if (!map_water_determine_orientation_size3(x, y, 1, 0, 0)) {
            Data_State.selectedBuilding.drawAsConstructing = 1;
        }
    } else if (Data_State.selectedBuilding.meadowRequired) {
        Terrain_existsTileWithinRadiusWithType(x, y, 3, 1, TERRAIN_MEADOW);
    } else if (Data_State.selectedBuilding.rockRequired) {
        Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_ROCK);
    } else if (Data_State.selectedBuilding.treesRequired) {
        Terrain_existsTileWithinRadiusWithType(x, y, 2, 1, TERRAIN_TREE | TERRAIN_SCRUB);
    } else if (Data_State.selectedBuilding.waterRequired) {
        Terrain_existsTileWithinRadiusWithType(x, y, 2, 3, TERRAIN_WATER);
    } else if (Data_State.selectedBuilding.wallRequired) {
        Terrain_allTilesWithinRadiusHaveType(x, y, 2, 0, TERRAIN_WALL);
    } else {
        if (!(type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) &&
            !(type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS) > 0) &&
            !(type == BUILDING_DISTRIBUTION_CENTER_UNUSED && Data_CityInfo.buildingDistributionCenterPlaced)) {
            int size = building_properties_for_type(type)->size;
            Terrain_updateToPlaceBuildingToOverlay(size, x, y, TERRAIN_ALL, 0);
        }
    }
    data.cost = current_cost;
}

void building_construction_place(int orientation)
{
    data.in_progress = 0;
    int x_start = data.x_start;
    int y_start = data.y_start;
    int x_end = data.x_end;
    int y_end = data.y_end;
    building_type type = data.type;
    building_construction_warning_reset();
    if (!type) {
        return;
    }
    if (city_finance_out_of_money()) {
        map_property_clear_constructing_and_deleted();
        city_warning_show(WARNING_OUT_OF_MONEY);
        return;
    }
    if (type >= BUILDING_LARGE_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
        map_property_clear_constructing_and_deleted();
        city_warning_show(WARNING_MARBLE_NEEDED_LARGE_TEMPLE);
        return;
    }
    if (type == BUILDING_ORACLE && Data_CityInfo.resourceStored[RESOURCE_MARBLE] < 2) {
        map_property_clear_constructing_and_deleted();
        city_warning_show(WARNING_MARBLE_NEEDED_ORACLE);
        return;
    }
    if (type != BUILDING_CLEAR_LAND && Figure_hasNearbyEnemy(x_start, y_start, x_end, y_end)) {
        if (type == BUILDING_WALL || type == BUILDING_ROAD || type == BUILDING_AQUEDUCT) {
            game_undo_restore_map(0);
        } else if (type == BUILDING_PLAZA || type == BUILDING_GARDENS) {
            game_undo_restore_map(1);
        } else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
            map_bridge_reset_building_length();
        } else {
            map_property_clear_constructing_and_deleted();
        }
        city_warning_show(WARNING_ENEMY_NEARBY);
        return;
    }

    int placement_cost = model_get_building(type)->cost;
    if (type == BUILDING_CLEAR_LAND) {
        // BUG: if confirmation has to be asked (bridge/fort), the previous cost is deducted from treasury
        // and if user chooses 'no', they still pay for removal
        int items_placed = building_construction_clear_land(0, x_start, y_start, x_end, y_end);
        if (items_placed >= 0) {
            items_placed = last_items_cleared;
        }
        placement_cost *= items_placed;
    } else if (type == BUILDING_WALL) {
        placement_cost *= place_wall(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_ROAD) {
        placement_cost *= place_road(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_PLAZA) {
        placement_cost *= place_plaza(0, x_start, y_start, x_end, y_end);
    } else if (type == BUILDING_GARDENS) {
        placement_cost *= place_garden(x_start, y_start, x_end, y_end);
        map_routing_update_land();
    } else if (type == BUILDING_LOW_BRIDGE) {
        int length = map_bridge_add(x_end, y_end, 0);
        if (length <= 1) {
            city_warning_show(WARNING_SHORE_NEEDED);
            return;
        }
        placement_cost *= length;
    } else if (type == BUILDING_SHIP_BRIDGE) {
        int length = map_bridge_add(x_end, y_end, 1);
        if (length <= 1) {
            city_warning_show(WARNING_SHORE_NEEDED);
            return;
        }
        placement_cost *= length;
    } else if (type == BUILDING_AQUEDUCT) {
        int cost;
        if (!place_aqueduct(0, x_start, y_start, x_end, y_end, &cost)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return;
        }
        placement_cost = cost;
        TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
        map_routing_update_land();
    } else if (type == BUILDING_DRAGGABLE_RESERVOIR) {
        struct reservoir_info info;
        if (!place_reservoir_and_aqueducts(0, x_start, y_start, x_end, y_end, &info)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED);
            return;
        }
        if (info.place_reservoir_at_start == PlaceReservoir_Yes) {
            building *reservoir = building_create(BUILDING_RESERVOIR, x_start - 1, y_start - 1);
            game_undo_add_building(reservoir);
            Terrain_addBuildingToGrids(reservoir->id, x_start-1, y_start-1, 3, image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
            map_aqueduct_set(map_grid_offset(x_start-1, y_start-1), 0);
        }
        if (info.place_reservoir_at_end == PlaceReservoir_Yes) {
            building *reservoir = building_create(BUILDING_RESERVOIR, x_end - 1, y_end - 1);
            game_undo_add_building(reservoir);
            Terrain_addBuildingToGrids(reservoir->id, x_end-1, y_end-1, 3, image_group(GROUP_BUILDING_RESERVOIR), TERRAIN_BUILDING);
            map_aqueduct_set(map_grid_offset(x_end-1, y_end-1), 0);
            if (!map_terrain_exists_tile_in_area_with_type(x_start - 2, y_start - 2, 5, TERRAIN_WATER) && info.place_reservoir_at_start == PlaceReservoir_No) {
                building_construction_warning_check_reservoir(BUILDING_RESERVOIR);
            }
        }
        placement_cost = info.cost;
        TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);
        map_routing_update_land();
    } else if (type == BUILDING_HOUSE_VACANT_LOT) {
        placement_cost *= place_houses(0, x_start, y_start, x_end, y_end);
    } else if (!building_construction_place_building(type, x_end, y_end)) {
        return;
    }
    if ((type >= BUILDING_LARGE_TEMPLE_CERES && type <= BUILDING_LARGE_TEMPLE_VENUS) || type == BUILDING_ORACLE) {
        building_warehouses_remove_resource(RESOURCE_MARBLE, 2);
    }
    formation_move_herds_away(x_end, y_end);
    city_finance_process_construction(placement_cost);
    if (type != BUILDING_TRIUMPHAL_ARCH) {
        game_undo_finish_build(placement_cost);
    }
}
