#include "undo.h"

#include "building/house.h"
#include "building/construction.h"
#include "building/industry.h"
#include "building/menu.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "core/image.h"
#include "figure/roamer_preview.h"
#include "game/resource.h"
#include "graphics/window.h"
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

int game_can_undo(void)
{
    return data.ready && data.available;
}

void game_undo_disable(void)
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

void game_undo_adjust_building(building *b)
{
    for (int i = 0; i < MAX_UNDO_BUILDINGS; i++) {
        if (data.buildings[i].id == b->id) {
            // found! update the building now
            memcpy(&data.buildings[i], b, sizeof(building));
        }
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

static void clear_buildings(void)
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
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_UNDO) {
            data.available = 0;
            return 0;
        }
        if (b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
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

void game_undo_restore_building_state(void)
{
    for (int i = 0; i < data.num_buildings; i++) {
        if (data.buildings[i].id) {
            building *b = building_get(data.buildings[i].id);
            if (b->state == BUILDING_STATE_DELETED_BY_PLAYER) {
                b->state = BUILDING_STATE_IN_USE;
            }
            b->is_deleted = 0;
        }
    }
    clear_buildings();
}

static void restore_map_images(void)
{
    int map_width, map_height;
    map_grid_size(&map_width, &map_height);
    for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
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
    window_invalidate();
}

static void add_building_to_terrain(building *b)
{
    if (b->id <= 0) {
        return;
    }
    if (building_is_farm(b->type)) {
        int image_offset;
        switch (b->type) {
            default:
            case BUILDING_WHEAT_FARM: image_offset = 0; break;
            case BUILDING_VEGETABLE_FARM: image_offset = 5; break;
            case BUILDING_FRUIT_FARM: image_offset = 10; break;
            case BUILDING_OLIVE_FARM: image_offset = 15; break;
            case BUILDING_VINES_FARM: image_offset = 20; break;
            case BUILDING_PIG_FARM: image_offset = 25; break;
        }
        map_building_tiles_add_farm(b->id, b->x, b->y,
            image_group(GROUP_BUILDING_FARM_CROPS) + image_offset, 0);
    } else {
        int size = building_properties_for_type(b->type)->size;
        if (building_is_house(b->type) && b->house_is_merged) {
            size = 2;
        }
        map_building_tiles_add(b->id, b->x, b->y, size, 0, 0);
        if (b->type == BUILDING_WHARF) {
            b->data.industry.fishing_boat_id = 0;
        }
    }
    b->state = BUILDING_STATE_IN_USE;
}

void game_undo_perform(void)
{
    if (!game_can_undo()) {
        return;
    }
    data.available = 0;
    city_finance_process_construction(-data.building_cost);
    if (data.type == BUILDING_CLEAR_LAND) {
        for (int i = 0; i < data.num_buildings; i++) {
            if (data.buildings[i].id) {
                building *b = building_restore_from_undo(&data.buildings[i]);
                switch (b->type) {
                    default:
                        break;
                    case BUILDING_WAREHOUSE:
                    case BUILDING_GRANARY:
                        if (!building_storage_restore(b->storage_id)) {
                            building_storage_reset_building_ids();
                        }
                        break;
                    case BUILDING_SENATE_UPGRADED:
                        city_buildings_add_senate(b);
                        break;
                    case BUILDING_DOCK:
                        city_buildings_add_dock();
                        break;
                    case BUILDING_BARRACKS:
                        city_buildings_add_barracks(b);
                        break;
                    case BUILDING_DISTRIBUTION_CENTER_UNUSED:
                        city_buildings_add_distribution_center(b);
                        break;
                    case BUILDING_HIPPODROME:
                        city_buildings_add_hippodrome();
                        break;
                    case BUILDING_TRIUMPHAL_ARCH:
                        city_buildings_build_triumphal_arch();
                        building_menu_update();
                        if (building_construction_type() == BUILDING_TRIUMPHAL_ARCH && !building_menu_is_enabled(BUILDING_TRIUMPHAL_ARCH)) {
                            building_construction_clear_type();
                        }
                        break;
                    case BUILDING_MESS_HALL:
                        city_buildings_add_mess_hall(b);
                        break;
                }
                if (building_is_house(b->type)) {
                    building_house_restore_population_after_undo(b);
                }
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
                building_get(data.buildings[i].id)->state = BUILDING_STATE_UNDO;
            }
        }
    }
    map_routing_update_land();
    map_routing_update_walls();
    figure_roamer_preview_reset(building_construction_type());
    data.num_buildings = 0;
}

void game_undo_reduce_time_available(void)
{
    if (!game_can_undo()) {
        return;
    }
    if (data.timeout_ticks <= 0 || scenario_earthquake_is_in_progress()) {
        data.available = 0;
        clear_buildings();
        window_invalidate();
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
        window_invalidate();
        return;
    }
    if (data.type == BUILDING_HOUSE_VACANT_LOT) {
        for (int i = 0; i < data.num_buildings; i++) {
            if (data.buildings[i].id && building_get(data.buildings[i].id)->house_population) {
                // no undo on a new house where people moved in
                data.available = 0;
                window_invalidate();
                return;
            }
        }
    }
    for (int i = 0; i < data.num_buildings; i++) {
        if (data.buildings[i].id) {
            building *b = building_get(data.buildings[i].id);
            if (b->state == BUILDING_STATE_UNDO ||
                b->state == BUILDING_STATE_RUBBLE ||
                b->state == BUILDING_STATE_DELETED_BY_GAME) {
                data.available = 0;
                window_invalidate();
                return;
            }
            if (b->type != data.buildings[i].type || b->grid_offset != data.buildings[i].grid_offset) {
                data.available = 0;
                window_invalidate();
                return;
            }
        }
    }
}
