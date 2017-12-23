#include "construction_clear.h"

#include "building/building.h"
#include "city/warning.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

#include "Data/State.h"
#include "../HousePopulation.h"
#include "../TerrainGraphics.h"
#include "../Undo.h"
#include "UI/PopupDialog.h"
#include "UI/Window.h"

static struct {
    int x_start;
    int y_start;
    int x_end;
    int y_end;
    int bridge_confirmed;
    int fort_confirmed;
} confirm;

static int clear_land_confirmed(int measureOnly, int x_start, int y_start, int x_end, int y_end)
{
    int items_placed = 0;
    Undo_restoreBuildings();
    map_terrain_restore();
    map_aqueduct_restore();
    Undo_restoreTerrainGraphics();

    int x_min, x_max, y_min, y_max;
    map_grid_start_end_to_area(x_start, y_start, x_end, y_end, &x_min, &y_min, &x_max, &y_max);

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            int gridOffset = map_grid_offset(x,y);
            if (map_terrain_is(gridOffset, TERRAIN_ROCK | TERRAIN_ELEVATION)) {
                continue;
            }
            if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
                int buildingId = map_building_at(gridOffset);
                if (!buildingId) {
                    continue;
                }
                building *b = building_get(buildingId);
                if (b->type == BUILDING_BURNING_RUIN || b->type == BUILDING_NATIVE_CROPS ||
                    b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
                    continue;
                }
                if (b->state == BuildingState_DeletedByPlayer) {
                    continue;
                }
                if (b->type == BUILDING_FORT_GROUND || b->type == BUILDING_FORT) {
                    if (!measureOnly && confirm.fort_confirmed != 1) {
                        continue;
                    }
                    if (!measureOnly && confirm.fort_confirmed == 1) {
                        Data_State.undoAvailable = 0;
                    }
                }
                if (b->houseSize && b->housePopulation && !measureOnly) {
                    HousePopulation_createHomeless(b->x, b->y, b->housePopulation);
                    b->housePopulation = 0;
                }
                if (b->state != BuildingState_DeletedByPlayer) {
                    items_placed++;
                    Undo_addBuildingToList(buildingId);
                }
                b->state = BuildingState_DeletedByPlayer;
                b->isDeleted = 1;
                building *space = b;
                for (int i = 0; i < 9; i++) {
                    if (space->prevPartBuildingId <= 0) {
                        break;
                    }
                    int spaceId = space->prevPartBuildingId;
                    space = building_get(spaceId);
                    Undo_addBuildingToList(spaceId);
                    space->state = BuildingState_DeletedByPlayer;
                }
                space = b;
                for (int i = 0; i < 9; i++) {
                    space = building_next(space);
                    if (space->id <= 0) {
                        break;
                    }
                    Undo_addBuildingToList(space->id);
                    space->state = BuildingState_DeletedByPlayer;
                }
            } else if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT)) {
                map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
                items_placed++;
                map_aqueduct_remove(gridOffset);
            } else if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
                if (!measureOnly && map_bridge_count_figures(gridOffset) > 0) {
                    city_warning_show(WARNING_PEOPLE_ON_BRIDGE);
                } else if (confirm.bridge_confirmed == 1) {
                    map_bridge_remove(gridOffset, measureOnly);
                    items_placed++;
                }
            } else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
                map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
                items_placed++;
            }
        }
    }
    int radius;
    if (x_max - x_min <= y_max - y_min) {
        radius = y_max - y_min + 3;
    } else {
        radius = x_max - x_min + 3;
    }
    TerrainGraphics_updateRegionEmptyLand(x_min, y_min, x_max, y_max);
    TerrainGraphics_updateRegionMeadow(x_min, y_min, x_max, y_max);
    TerrainGraphics_updateRegionRubble(x_min, y_min, x_max, y_max);
    TerrainGraphics_updateAllGardens();
    TerrainGraphics_updateAreaRoads(x_min, y_min, radius);
    TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
    TerrainGraphics_updateAreaWalls(x_min, y_min, radius);
    if (!measureOnly) {
        map_routing_update_land();
        map_routing_update_walls();
        map_routing_update_water();
        UI_Window_requestRefresh();
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
            int gridOffset = map_grid_offset(x,y);
            int buildingId = map_building_at(gridOffset);
            if (buildingId) {
                building *b = building_get(buildingId);
                if (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND) {
                    ask_confirm_fort = 1;
                }
            }
            if (map_is_bridge(gridOffset)) {
                ask_confirm_bridge = 1;
            }
        }
    }
    confirm.x_start = x_start;
    confirm.y_start = y_start;
    confirm.x_end = x_end;
    confirm.y_end = y_end;
    if (ask_confirm_fort) {
        UI_PopupDialog_show(PopupDialog_DeleteFort, confirm_delete_fort, 2);
        return -1;
    } else if (ask_confirm_bridge) {
        UI_PopupDialog_show(PopupDialog_DeleteBridge, confirm_delete_bridge, 2);
        return -1;
    } else {
        return clear_land_confirmed(measure_only, x_start, y_start, x_end, y_end);
    }
}
