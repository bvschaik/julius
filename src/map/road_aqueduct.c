#include "road_aqueduct.h"

#include "core/direction.h"
#include "graphics/image.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/routing.h"

#include "Data/State.h"

int map_can_place_road_under_aqueduct(int gridOffset)
{
    int graphic = map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT);
    int checkRoadY;
    switch (graphic) {
        case 0:
        case 2:
        case 8:
        case 15:
        case 17:
        case 23:
            checkRoadY = 1;
            break;
        case 1:
        case 3:
        case 9: case 10: case 11: case 12: case 13: case 14:
        case 16:
        case 18:
        case 24: case 25: case 26: case 27: case 28: case 29:
            checkRoadY = 0;
            break;
        default: // not a straight aqueduct
            return 0;
    }
    if (Data_State.map.orientation == DIR_6_LEFT || Data_State.map.orientation == DIR_2_RIGHT) {
        checkRoadY = !checkRoadY;
    }
    if (checkRoadY) {
        int dy_up = map_grid_delta(0, -1);
        int dy_down = map_grid_delta(0, 1);
        if ((Data_Grid_terrain[gridOffset + dy_up] & Terrain_Road) ||
            map_routing_distance(gridOffset + dy_up) > 0) {
            return 0;
        }
        if ((Data_Grid_terrain[gridOffset + dy_down] & Terrain_Road) ||
            map_routing_distance(gridOffset + dy_down) > 0) {
            return 0;
        }
    } else {
        int dx_left = map_grid_delta(-1, 0);
        int dx_right = map_grid_delta(1, 0);
        if ((Data_Grid_terrain[gridOffset + dx_left] & Terrain_Road) ||
            map_routing_distance(gridOffset + dx_left) > 0) {
            return 0;
        }
        if ((Data_Grid_terrain[gridOffset + dx_right] & Terrain_Road) ||
            map_routing_distance(gridOffset + dx_right) > 0) {
            return 0;
        }
    }
    return 1;
}

int map_can_place_aqueduct_on_road(int gridOffset)
{
    int graphic = map_image_at(gridOffset) - image_group(GROUP_TERRAIN_ROAD);
    if (graphic != 0 && graphic != 1 && graphic != 49 && graphic != 50) {
        return 0;
    }
    int checkRoadY = graphic == 0 || graphic == 49;
    if (Data_State.map.orientation == DIR_6_LEFT || Data_State.map.orientation == DIR_2_RIGHT) {
        checkRoadY = !checkRoadY;
    }
    if (checkRoadY) {
        if (map_routing_distance(gridOffset + map_grid_delta(0, -1)) > 0 ||
            map_routing_distance(gridOffset + map_grid_delta(0, 1)) > 0) {
            return 0;
        }
    } else {
        if (map_routing_distance(gridOffset + map_grid_delta(-1, 0)) > 0 ||
            map_routing_distance(gridOffset + map_grid_delta(1, 0)) > 0) {
            return 0;
        }
    }
    return 1;
}

int map_get_aqueduct_with_road_image(int gridOffset)
{
    int graphic = map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT);
    switch (graphic) {
        case 2:
            return 8;
        case 3:
            return 9;
        case 0:
        case 1:
        case 8:
        case 9:
        case 15:
        case 16:
        case 17:
        case 18:
        case 23:
        case 24:
            // unchanged
            return graphic;
        default:
            // shouldn't happen
            return 8;
    }
}
