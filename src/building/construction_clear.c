#include "construction_clear.h"

#include "building/building.h"
#include "city/warning.h"
#include "core/config.h"
#include "figuretype/migrant.h"
#include "game/undo.h"
#include "graphics/window.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "window/popup_dialog.h"

static struct {
    int x_start;
    int y_start;
    int x_end;
    int y_end;
    int bridge_confirmed;
    int fort_confirmed;
} confirm;

static building *get_deletable_building(int grid_offset)
{
    int building_id = map_building_at(grid_offset);
    if (!building_id) {
        return 0;
    }
    building *b = building_main(building_get(building_id));
    if (b->type == BUILDING_BURNING_RUIN || b->type == BUILDING_NATIVE_CROPS ||
        b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
        return 0;
    }
    if (b->state == BUILDING_STATE_DELETED_BY_PLAYER || b->is_deleted) {
        return 0;
    }
    return b;
}

static int clear_land_confirmed(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    int items_placed = 0;
    game_undo_restore_building_state();
    game_undo_restore_map(0);

    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int visual_feedback_on_delete = config_get(CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE);

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x,y);
            if (map_terrain_is(grid_offset, TERRAIN_ROCK | TERRAIN_ELEVATION)) {
                continue;
            }
            if (measure_only && visual_feedback_on_delete) {
                building *b = get_deletable_building(grid_offset);
                if (map_property_is_deleted(grid_offset) || (b && map_property_is_deleted(b->grid_offset))) {
                    continue;
                }
                map_building_tiles_mark_deleting(grid_offset);
                if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
                    if (b) {
                        items_placed++;
                    }
                } else if (map_terrain_is(grid_offset, TERRAIN_WATER)) { // keep the "bridge is free" bug from C3
                    continue;
                } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT) || map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                    items_placed++;
                }
                continue;
            }
            if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
                building *b = get_deletable_building(grid_offset);
                if (!b) {
                    continue;
                }
                if (b->type == BUILDING_FORT_GROUND || b->type == BUILDING_FORT) {
                    if (!measure_only && confirm.fort_confirmed != 1) {
                        continue;
                    }
                    if (!measure_only && confirm.fort_confirmed == 1) {
                        game_undo_disable();
                    }
                }
                if (b->house_size && b->house_population && !measure_only) {
                    figure_create_homeless(b->x, b->y, b->house_population);
                    b->house_population = 0;
                }
                if (b->state != BUILDING_STATE_DELETED_BY_PLAYER) {
                    items_placed++;
                    game_undo_add_building(b);
                }
                b->state = BUILDING_STATE_DELETED_BY_PLAYER;
                b->is_deleted = 1;
                building *space = b;
                for (int i = 0; i < 9; i++) {
                    if (space->prev_part_building_id <= 0) {
                        break;
                    }
                    space = building_get(space->prev_part_building_id);
                    game_undo_add_building(space);
                    space->state = BUILDING_STATE_DELETED_BY_PLAYER;
                }
                space = b;
                for (int i = 0; i < 9; i++) {
                    space = building_next(space);
                    if (space->id <= 0) {
                        break;
                    }
                    game_undo_add_building(space);
                    space->state = BUILDING_STATE_DELETED_BY_PLAYER;
                }
            } else if (map_terrain_is(grid_offset, TERRAIN_AQUEDUCT)) {
                map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
                items_placed++;
                map_aqueduct_remove(grid_offset);
            } else if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
                if (!measure_only && map_bridge_count_figures(grid_offset) > 0) {
                    city_warning_show(WARNING_PEOPLE_ON_BRIDGE);
                } else if (confirm.bridge_confirmed == 1) {
                    map_bridge_remove(grid_offset, measure_only);
                    items_placed++;
                }
            } else if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
                if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
                    map_property_clear_plaza_or_earthquake(grid_offset);
                }
                map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
                items_placed++;
            }
        }
    }
    if (!measure_only || !visual_feedback_on_delete) {
        int radius;
        if (x_max - x_min <= y_max - y_min) {
            radius = y_max - y_min + 3;
        } else {
            radius = x_max - x_min + 3;
        }
        map_tiles_update_region_empty_land(x_min, y_min, x_max, y_max);
        map_tiles_update_region_meadow(x_min, y_min, x_max, y_max);
        map_tiles_update_region_rubble(x_min, y_min, x_max, y_max);
        map_tiles_update_all_gardens();
        map_tiles_update_area_roads(x_min, y_min, radius);
        map_tiles_update_all_plazas();
        map_tiles_update_area_walls(x_min, y_min, radius);
        map_tiles_update_region_aqueducts(x_min - 3, y_min - 3, x_max + 3, y_max + 3);
    }
    if (!measure_only) {
        map_routing_update_land();
        map_routing_update_walls();
        map_routing_update_water();
        if (config_get(CONFIG_GP_CH_IMMEDIATELY_DELETE_BUILDINGS)) {
            building_update_state();
        }
        window_invalidate();
    }
    return items_placed;
}

static void confirm_delete_fort(int accepted)
{
    if (accepted == 1) {
        confirm.fort_confirmed = 1;
    } else {
        confirm.fort_confirmed = -1;
    }
    clear_land_confirmed(0, confirm.x_start, confirm.y_start, confirm.x_end, confirm.y_end);
}

static void confirm_delete_bridge(int accepted)
{
    if (accepted == 1) {
        confirm.bridge_confirmed = 1;
    } else {
        confirm.bridge_confirmed = -1;
    }
    clear_land_confirmed(0, confirm.x_start, confirm.y_start, confirm.x_end, confirm.y_end);
}

int building_construction_clear_land(int measure_only, int x_start, int y_start, int x_end, int y_end)
{
    confirm.fort_confirmed = 0;
    confirm.bridge_confirmed = 0;
    if (measure_only) {
        return clear_land_confirmed(measure_only, x_start, y_start, x_end, y_end);
    }

    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    int ask_confirm_bridge = 0;
    int ask_confirm_fort = 0;
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int grid_offset = map_grid_offset(x,y);
            int building_id = map_building_at(grid_offset);
            if (building_id) {
                building *b = building_get(building_id);
                if (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND) {
                    ask_confirm_fort = 1;
                }
            }
            if (map_is_bridge(grid_offset)) {
                ask_confirm_bridge = 1;
            }
        }
    }
    confirm.x_start = x_start;
    confirm.y_start = y_start;
    confirm.x_end = x_end;
    confirm.y_end = y_end;
    if (ask_confirm_fort) {
        window_popup_dialog_show(POPUP_DIALOG_DELETE_FORT, confirm_delete_fort, 2);
        return -1;
    } else if (ask_confirm_bridge) {
        window_popup_dialog_show(POPUP_DIALOG_DELETE_BRIDGE, confirm_delete_bridge, 2);
        return -1;
    } else {
        return clear_land_confirmed(measure_only, x_start, y_start, x_end, y_end);
    }
}
