#include "undo.h"

#include "building/industry.h"
#include "building/properties.h"
#include "building/warehouse.h"
#include "city/finance.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "scenario/earthquake.h"

#include "Data/State.h"
#include "UI/Window.h"

#include <string.h>

#define MAX_UNDO_BUILDINGS 50

static struct {
    int available;
    int ready;
    int timeout_ticks;
    int building_cost;
    int num_buildings;
    building_type type;
    building buildings[MAX_UNDO_BUILDINGS];
} data;

int game_can_undo()
{
    return data.ready && data.available;
}

void game_undo_disable()
{
    data.available = 0;
}

void game_undo_add_building(building *b)
{
    if (b->id <= 0) {
        return;
    }
    data.num_buildings = 0;
    int is_on_list = 0;
    for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
        if (data.buildings[i].id) {
            data.num_buildings++;
        }
        if (data.buildings[i].id == b->id) {
            is_on_list = 1;
        }
    }
    if (!is_on_list) {
        for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
            if (!data.buildings[i].id) {
                data.num_buildings++;
                memcpy(&data.buildings[i], b, sizeof(building));
                return;
            }
        }
        data.available = 0;
    }
}

int game_undo_contains_building(int building_id)
{
    if (building_id <= 0 || !game_can_undo()) {
        return 0;
    }
    if (data.num_buildings <= 0) {
        return 0;
    }
    for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
        if (data.buildings[i].id == building_id) {
            return 1;
        }
    }
    return 0;
}

static void clear_buildings()
{
    data.num_buildings = 0;
    memset(data.buildings, 0, MAX_UNDO_BUILDINGS * sizeof(building));
}

int game_undo_start_build(building_type type)
{
    data.ready = 0;
    data.available = 1;
    data.timeout_ticks = 0;
    data.building_cost = 0;
    data.type = type;
    clear_buildings();
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BuildingState_Undo) {
            data.available = 0;
            return 0;
        }
        if (b->state == BuildingState_DeletedByPlayer) {
            data.available = 0;
        }
    }

    map_image_backup();
    map_terrain_backup();
    map_aqueduct_backup();
    map_property_backup();
    map_sprite_backup();

    return 1;
}

void game_undo_restore_building_state()
{
    for (int i = 0; i < data.num_buildings; i++) {
        if (data.buildings[i].id) {
            building *b = building_get(data.buildings[i].id);
            if (b->state == BuildingState_DeletedByPlayer) {
                b->state = BuildingState_InUse;
            }
            b->isDeleted = 0;
        }
    }
    clear_buildings();
}

static void restore_map_images()
{
    for (int y = 0; y < Data_State.map.height; y++) {
        for (int x = 0; x < Data_State.map.width; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (!map_building_at(grid_offset)) {
                map_image_restore_at(grid_offset);
            }
        }
    }
}

void game_undo_restore_map(int include_properties)
{
    map_terrain_restore();
    map_aqueduct_restore();
    if (include_properties) {
        map_property_restore();
    }
    restore_map_images();
}

void game_undo_finish_build(int cost)
{
    data.ready = 1;
    data.timeout_ticks = 500;
    data.building_cost = cost;
    UI_Window_requestRefresh();
}

static void add_building_to_terrain(building *b)
{
    if (b->id <= 0) {
        return;
    }
    if (building_is_farm(b->type)) {
        int graphicOffset;
        switch (b->type) {
            default:
            case BUILDING_WHEAT_FARM: graphicOffset = 0; break;
            case BUILDING_VEGETABLE_FARM: graphicOffset = 5; break;
            case BUILDING_FRUIT_FARM: graphicOffset = 10; break;
            case BUILDING_OLIVE_FARM: graphicOffset = 15; break;
            case BUILDING_VINES_FARM: graphicOffset = 20; break;
            case BUILDING_PIG_FARM: graphicOffset = 25; break;
        }
        map_building_tiles_add_farm(b->id, b->x, b->y,
            image_group(GROUP_BUILDING_FARM_CROPS) + graphicOffset, 0);
    } else {
        int size = building_properties_for_type(b->type)->size;
        map_building_tiles_add(b->id, b->x, b->y, size, 0, 0);
        if (b->type == BUILDING_WHARF) {
            b->data.other.boatFigureId = 0;
        }
    }
    b->state = BuildingState_InUse;
}

void game_undo_perform()
{
    if (!game_can_undo()) {
        return;
    }
    data.available = 0;
    city_finance_process_construction(-data.building_cost);
    if (data.type == BUILDING_CLEAR_LAND) {
        for (int i = 0; i < data.num_buildings; i++) {
            if (data.buildings[i].id) {
                building *b = building_get(data.buildings[i].id);
                memcpy(b, &data.buildings[i], sizeof(building));
                add_building_to_terrain(b);
            }
        }
        map_terrain_restore();
        map_aqueduct_restore();
        map_sprite_restore();
        map_image_restore();
        map_property_restore();
        map_property_clear_constructing_and_deleted();
    } else if (data.type == BUILDING_AQUEDUCT || data.type == BUILDING_ROAD ||
            data.type == BUILDING_WALL) {
        map_terrain_restore();
        map_aqueduct_restore();
        restore_map_images();
    } else if (data.type == BUILDING_LOW_BRIDGE || data.type == BUILDING_SHIP_BRIDGE) {
        map_terrain_restore();
        map_sprite_restore();
        restore_map_images();
    } else if (data.type == BUILDING_PLAZA || data.type == BUILDING_GARDENS) {
        map_terrain_restore();
        map_aqueduct_restore();
        map_property_restore();
        restore_map_images();
    } else if (data.num_buildings) {
        if (data.type == BUILDING_DRAGGABLE_RESERVOIR) {
            map_terrain_restore();
            map_aqueduct_restore();
            restore_map_images();
        }
        for (int i = 0; i < data.num_buildings; i++) {
            if (data.buildings[i].id) {
                building *b = building_get(data.buildings[i].id);
                if (b->type == BUILDING_ORACLE || (b->type >= BUILDING_LARGE_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS)) {
                    building_warehouses_add_resource(RESOURCE_MARBLE, 2);
                }
                b->state = BuildingState_Undo;
            }
        }
    }
    map_routing_update_land();
    map_routing_update_walls();
    data.num_buildings = 0;
}

void game_undo_reduce_time_available()
{
    if (!game_can_undo()) {
        return;
    }
    if (data.timeout_ticks <= 0 || scenario_earthquake_is_in_progress()) {
        data.available = 0;
        clear_buildings();
        UI_Window_requestRefresh();
        return;
    }
    data.timeout_ticks--;
    switch (data.type) {
        case BUILDING_CLEAR_LAND:
        case BUILDING_AQUEDUCT:
        case BUILDING_ROAD:
        case BUILDING_WALL:
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
            return;
        default: break;
    }
    if (data.num_buildings <= 0) {
        data.available = 0;
        UI_Window_requestRefresh();
        return;
    }
    if (data.type == BUILDING_HOUSE_VACANT_LOT) {
        for (int i = 0; i < data.num_buildings; i++) {
            if (data.buildings[i].id && building_get(data.buildings[i].id)->housePopulation) {
                // no undo on a new house where people moved in
                data.available = 0;
                UI_Window_requestRefresh();
                return;
            }
        }
    }
    for (int i = 0; i < data.num_buildings; i++) {
        if (data.buildings[i].id) {
            building *b = building_get(data.buildings[i].id);
            if (b->state == BuildingState_Undo ||
                b->state == BuildingState_Rubble ||
                b->state == BuildingState_DeletedByGame) {
                data.available = 0;
                UI_Window_requestRefresh();
                return;
            }
            if (b->type != data.buildings[i].type || b->gridOffset != data.buildings[i].gridOffset) {
                data.available = 0;
                UI_Window_requestRefresh();
                return;
            }
        }
    }
}
