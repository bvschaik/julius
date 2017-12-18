#include "industry.h"

#include "core/calc.h"
#include "game/resource.h"
#include "graphics/image.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#define MAX_PROGRESS_RAW 200
#define MAX_PROGRESS_WORKSHOP 400

int building_is_farm(building_type type)
{
    return type >= BUILDING_WHEAT_FARM && type <= BUILDING_PIG_FARM;
}

int building_is_workshop(building_type type)
{
    return type >= BUILDING_WINE_WORKSHOP && type <= BUILDING_POTTERY_WORKSHOP;
}

static int max_progress(const building *b)
{
    return b->subtype.workshopType ? MAX_PROGRESS_WORKSHOP : MAX_PROGRESS_RAW;
}

static void update_farm_image(const building *b)
{
    TerrainGraphics_setBuildingFarm(b->id, b->x, b->y,
        image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->outputResourceId - 1),
        b->data.industry.progress);
}

void building_industry_update_production()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !b->outputResourceId) {
            continue;
        }
        b->data.industry.hasFullResource = 0;
        if (b->housesCovered <= 0 || b->numWorkers <= 0) {
            continue;
        }
        if (b->subtype.workshopType && !b->loadsStored) {
            continue;
        }
        if (b->data.industry.curseDaysLeft) {
            b->data.industry.curseDaysLeft--;
        } else {
            if (b->data.industry.blessingDaysLeft) {
                b->data.industry.blessingDaysLeft--;
            }
            if (b->type == BUILDING_MARBLE_QUARRY) {
                b->data.industry.progress += b->numWorkers / 2;
            } else {
                b->data.industry.progress += b->numWorkers;
            }
            if (b->data.industry.blessingDaysLeft && building_is_farm(b->type)) {
                b->data.industry.progress += b->numWorkers;
            }
            int max = max_progress(b);
            if (b->data.industry.progress > max) {
                b->data.industry.progress = max;
            }
            if (building_is_farm(b->type)) {
                update_farm_image(b);
            }
        }
    }
}

void building_industry_update_wheat_production()
{
    if (scenario_property_climate() == CLIMATE_NORTHERN) {
        return;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !b->outputResourceId) {
            continue;
        }
        if (b->housesCovered <= 0 || b->numWorkers <= 0) {
            continue;
        }
        if (b->type == BUILDING_WHEAT_FARM && !b->data.industry.curseDaysLeft) {
            b->data.industry.progress += b->numWorkers;
            if (b->data.industry.blessingDaysLeft) {
                b->data.industry.progress += b->numWorkers;
            }
            if (b->data.industry.progress > MAX_PROGRESS_RAW) {
                b->data.industry.progress = MAX_PROGRESS_RAW;
            }
            update_farm_image(b);
        }
    }
}

int building_industry_has_produced_resource(building *b)
{
    return b->data.industry.progress >= max_progress(b);
}

void building_industry_start_new_production(building *b)
{
    b->data.industry.progress = 0;
    if (b->subtype.workshopType) {
        if (b->loadsStored) {
            if (b->loadsStored > 1) {
                b->data.industry.hasFullResource = 1;
            }
            b->loadsStored--;
        }
    }
    if (building_is_farm(b->type)) {
        update_farm_image(b);
    }
}

void building_bless_farms()
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->outputResourceId && building_is_farm(b->type)) {
            b->data.industry.progress = 200;
            b->data.industry.curseDaysLeft = 0;
            b->data.industry.blessingDaysLeft = 16;
            update_farm_image(b);
        }
    }
}

void building_curse_farms(int big_curse)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->outputResourceId && building_is_farm(b->type)) {
            b->data.industry.progress = 0;
            b->data.industry.blessingDaysLeft = 0;
            b->data.industry.curseDaysLeft = big_curse ? 48 : 4;
            update_farm_image(b);
        }
    }
}

void building_calculate_workshop_stocks()
{
    for (int i = 0; i < 6; i++) {
        Data_CityInfo.resourceWorkshopRawMaterialStored[i] = 0;
        Data_CityInfo.resourceWorkshopRawMaterialSpace[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !building_is_workshop(b->type)) {
            continue;
        }
        b->hasRoadAccess = 0;
        if (Terrain_hasRoadAccess(b->x, b->y, b->size, 0, 0)) {
            b->hasRoadAccess = 1;
            int room = 2 - b->loadsStored;
            if (room < 0) {
                room = 0;
            }
            int workshop_resource = b->subtype.workshopType;
            Data_CityInfo.resourceWorkshopRawMaterialSpace[workshop_resource] += room;
            Data_CityInfo.resourceWorkshopRawMaterialStored[workshop_resource] += b->loadsStored;
        }
    }
}

void building_workshop_add_raw_material(building *b)
{
    if (b->id > 0 && building_is_workshop(b->type)) {
        b->loadsStored++; // BUG: any raw material accepted
    }
}

int building_get_workshop_for_raw_material_with_room(int x, int y, int resource,
                                                     int distance_from_entry, int road_network_id,
                                                     int *x_dst, int *y_dst)
{
    if (Data_CityInfo.resourceStockpiled[resource]) {
        return 0;
    }
    int outputType;
    switch (resource) {
        case RESOURCE_OLIVES: outputType = WORKSHOP_OLIVES_TO_OIL; break;
        case RESOURCE_VINES: outputType = WORKSHOP_VINES_TO_WINE; break;
        case RESOURCE_IRON: outputType = WORKSHOP_IRON_TO_WEAPONS; break;
        case RESOURCE_TIMBER: outputType = WORKSHOP_TIMBER_TO_FURNITURE; break;
        case RESOURCE_CLAY: outputType = WORKSHOP_CLAY_TO_POTTERY; break;
        default: return 0;
    }
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !building_is_workshop(b->type)) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
            continue;
        }
        if (b->subtype.workshopType == outputType && b->roadNetworkId == road_network_id && b->loadsStored < 2) {
            int dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distanceFromEntry);
            if (b->loadsStored > 0) {
                dist += 20;
            }
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        *x_dst = min_building->roadAccessX;
        *y_dst = min_building->roadAccessY;
        return min_building->id;
    }
    return 0;
}

int building_get_workshop_for_raw_material(int x, int y, int resource,
                                           int distance_from_entry, int road_network_id,
                                           int *x_dst, int *y_dst)
{
    if (Data_CityInfo.resourceStockpiled[resource]) {
        return 0;
    }
    int outputType;
    switch (resource) {
        case RESOURCE_OLIVES: outputType = WORKSHOP_OLIVES_TO_OIL; break;
        case RESOURCE_VINES: outputType = WORKSHOP_VINES_TO_WINE; break;
        case RESOURCE_IRON: outputType = WORKSHOP_IRON_TO_WEAPONS; break;
        case RESOURCE_TIMBER: outputType = WORKSHOP_TIMBER_TO_FURNITURE; break;
        case RESOURCE_CLAY: outputType = WORKSHOP_CLAY_TO_POTTERY; break;
        default: return 0;
    }
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || !building_is_workshop(b->type)) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
            continue;
        }
        if (b->subtype.workshopType == outputType && b->roadNetworkId == road_network_id) {
            int dist = 10 * b->loadsStored +
                calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distanceFromEntry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        *x_dst = min_building->roadAccessX;
        *y_dst = min_building->roadAccessY;
        return min_building->id;
    }
    return 0;
}
