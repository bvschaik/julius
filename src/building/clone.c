#include "clone.h"

#include "building/building.h"
#include "building/construction.h"
#include "city/buildings.h"
#include "figure/formation.h"
#include "map/building.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"

/**
 * Takes a building and retrieve its proper type for cloning.
 * For example, given a fort, return the enumaration value corresponding to
 * the specific type of fort rather than the general value
 *
 * @param building Building to examine
 * @return the building_type value to clone, or BUILDING_NONE if not cloneable
 */
static short get_clone_type_from_building(building *building)
{
    short clone_type = building->type;

    if (building_is_house(clone_type)) {
        return BUILDING_HOUSE_VACANT_LOT;
    }

    switch (clone_type) {
        case BUILDING_RESERVOIR:
            return BUILDING_DRAGGABLE_RESERVOIR;
        case BUILDING_FORT:
            switch (building->subtype.fort_figure_type) {
                case FIGURE_FORT_LEGIONARY: return BUILDING_FORT_LEGIONARIES;
                case FIGURE_FORT_JAVELIN: return BUILDING_FORT_JAVELIN;
                case FIGURE_FORT_MOUNTED: return BUILDING_FORT_MOUNTED;
            }
            return BUILDING_NONE;
        case BUILDING_TRIUMPHAL_ARCH:
            // Triumphal arches don't seem to have any protection around making
            // more than you've earned, so check that here
            if (city_buildings_triumphal_arch_available()) {
                break;
            }
            // fallthrough
        case BUILDING_NATIVE_CROPS:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_BURNING_RUIN:
            return BUILDING_NONE;
    }

    return clone_type;
}

/**
 * Helper function for retrieving which construction mode to enter.
 *
 * @param grid_offset the grid_offset of the tile to examine
 * @return type to use in building_construction_set_type (0 if none)
 */
static int get_clone_type_from_grid_offset(int grid_offset)
{
    int terrain = map_terrain_get(grid_offset);

    if (terrain & TERRAIN_BUILDING) {
        int building_id = map_building_at(grid_offset);
        if (building_id) {
            building *building = building_main(building_get(building_id));
            return get_clone_type_from_building(building);
        }
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
    }

    return BUILDING_NONE;
}

/**
 * Enter construction mode with the same building as cursor is currently over
 */
void building_clone_from_grid_offset(int grid_offset)
{
    short clone_type = get_clone_type_from_grid_offset(grid_offset);

    if (clone_type) {
        building_construction_cancel();
        building_construction_set_type(clone_type);
    }
}
