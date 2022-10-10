#include "clone.h"

#include "building/building.h"
#include "figure/type.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"

/**
 * Takes a building and retrieve its proper type for cloning.
 * For example, given a fort, return the enumaration value corresponding to
 * the specific type of fort rather than the general value
 *
 * @param building Building to examine (can be null for destroyed building)
 * @param building_type Type of the building to clone (can be original building type before a fire)
 * @return the building_type value to clone, or BUILDING_NONE if not cloneable
 */
static building_type get_clone_type_from_building(building *b, building_type clone_type)
{
    if (building_is_house(clone_type)) {
        return BUILDING_HOUSE_VACANT_LOT;
    }

    switch (clone_type) {
        case BUILDING_RESERVOIR:
            return BUILDING_DRAGGABLE_RESERVOIR;
        case BUILDING_FORT:
            if (b) {
                switch (b->subtype.fort_figure_type) {
                    case FIGURE_FORT_LEGIONARY: return BUILDING_FORT_LEGIONARIES;
                    case FIGURE_FORT_JAVELIN: return BUILDING_FORT_JAVELIN;
                    case FIGURE_FORT_MOUNTED: return BUILDING_FORT_MOUNTED;
                }
            }
            return BUILDING_NONE;
        case BUILDING_NATIVE_CROPS:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
            return BUILDING_NONE;
        case BUILDING_BURNING_RUIN:
            if (b) {
                return get_clone_type_from_building(b, map_rubble_building_type(b->grid_offset));
            } else {
                return BUILDING_NONE;
            }
        case BUILDING_GARDEN_WALL_GATE:
        // Check neighbouring tiles to see if it's a part of garden wall.
        // If it is, return that garden wall type, otherwise return default garden wall type.
        {
            if (b) {
                int grid_offset = b->grid_offset;
                for (const int *tile_delta = map_grid_adjacent_offsets(b->size); *tile_delta; tile_delta++) {
                    building *neighbour = building_get(map_building_at(grid_offset + *tile_delta));
                    if (neighbour->type == BUILDING_ROOFED_GARDEN_WALL || neighbour->type == BUILDING_GARDEN_WALL) {
                        return neighbour->type;
                    }
                }
            }
            return BUILDING_GARDEN_WALL;
        }
        case BUILDING_HEDGE_GATE_LIGHT:
            return BUILDING_HEDGE_LIGHT;
        case BUILDING_HEDGE_GATE_DARK:
            return BUILDING_HEDGE_DARK;
        case BUILDING_PAVILION_BLUE:
        case BUILDING_PAVILION_GREEN:
        case BUILDING_PAVILION_ORANGE:
        case BUILDING_PAVILION_RED:
        case BUILDING_PAVILION_YELLOW:
            return BUILDING_PAVILION_BLUE;
        case BUILDING_PALISADE_GATE:
            return BUILDING_PALISADE;
        default:
            return clone_type;
    }
}

building_type building_clone_type_from_grid_offset(int grid_offset)
{
    int terrain = map_terrain_get(grid_offset);

    if (terrain & TERRAIN_BUILDING) {
        int building_id = map_building_at(grid_offset);
        if (building_id) {
            building *b = building_main(building_get(building_id));
            return get_clone_type_from_building(b, b->type);
        }
    } else if (terrain & TERRAIN_RUBBLE) {
        return get_clone_type_from_building(0, map_rubble_building_type(grid_offset));
    } else if (terrain & TERRAIN_AQUEDUCT) {
        return BUILDING_AQUEDUCT;
    } else if (terrain & TERRAIN_WALL) {
        return BUILDING_WALL;
    } else if (terrain & TERRAIN_GARDEN) {
        return BUILDING_GARDENS;
    } else if (terrain & TERRAIN_ROAD) {
        if (terrain & TERRAIN_WATER) {
            if (map_sprite_bridge_at(grid_offset) > 6) {
                return BUILDING_SHIP_BRIDGE;
            }
            return BUILDING_LOW_BRIDGE;
        } else if (map_property_is_plaza_or_earthquake(grid_offset)) {
            return BUILDING_PLAZA;
        }
        return BUILDING_ROAD;
    } else if (terrain & TERRAIN_HIGHWAY) {
        return BUILDING_HIGHWAY;
    }

    return BUILDING_NONE;
}
