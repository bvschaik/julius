#include "industry.h"

#include "city/resource.h"
#include "core/calc.h"
#include "core/image.h"
#include "game/resource.h"
#include "map/building_tiles.h"
#include "map/road_access.h"
#include "scenario/property.h"

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
    return b->subtype.workshop_type ? MAX_PROGRESS_WORKSHOP : MAX_PROGRESS_RAW;
}

static void update_farm_image(const building *b)
{
    map_building_tiles_add_farm(b->id, b->x, b->y,
        image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->output_resource_id - 1),
        b->data.industry.progress);
}

void building_industry_update_production(void)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->output_resource_id) {
            continue;
        }
        b->data.industry.has_raw_materials = 0;
        if (b->houses_covered <= 0 || b->num_workers <= 0) {
            continue;
        }
        if (b->subtype.workshop_type && !b->loads_stored) {
            continue;
        }
        if (b->data.industry.curse_days_left) {
            b->data.industry.curse_days_left--;
        } else {
            if (b->data.industry.blessing_days_left) {
                b->data.industry.blessing_days_left--;
            }
            if (b->type == BUILDING_MARBLE_QUARRY) {
                b->data.industry.progress += b->num_workers / 2;
            } else {
                b->data.industry.progress += b->num_workers;
            }
            if (b->data.industry.blessing_days_left && building_is_farm(b->type)) {
                b->data.industry.progress += b->num_workers;
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

void building_industry_update_wheat_production(void)
{
    if (scenario_property_climate() == CLIMATE_NORTHERN) {
        return;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->output_resource_id) {
            continue;
        }
        if (b->houses_covered <= 0 || b->num_workers <= 0) {
            continue;
        }
        if (b->type == BUILDING_WHEAT_FARM && !b->data.industry.curse_days_left) {
            b->data.industry.progress += b->num_workers;
            if (b->data.industry.blessing_days_left) {
                b->data.industry.progress += b->num_workers;
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
    if (b->subtype.workshop_type) {
        if (b->loads_stored) {
            if (b->loads_stored > 1) {
                b->data.industry.has_raw_materials = 1;
            }
            b->loads_stored--;
        }
    }
    if (building_is_farm(b->type)) {
        update_farm_image(b);
    }
}

void building_bless_farms(void)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->output_resource_id && building_is_farm(b->type)) {
            b->data.industry.progress = 200;
            b->data.industry.curse_days_left = 0;
            b->data.industry.blessing_days_left = 16;
            update_farm_image(b);
        }
    }
}

void building_curse_farms(int big_curse)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->output_resource_id && building_is_farm(b->type)) {
            b->data.industry.progress = 0;
            b->data.industry.blessing_days_left = 0;
            b->data.industry.curse_days_left = big_curse ? 48 : 4;
            update_farm_image(b);
        }
    }
}

void building_workshop_add_raw_material(building *b)
{
    if (b->id > 0 && building_is_workshop(b->type)) {
        b->loads_stored++; // BUG: any raw material accepted
    }
}

int building_get_workshop_for_raw_material_with_room(int x, int y, int resource,
                                                     int distance_from_entry, int road_network_id,
                                                     int *x_dst, int *y_dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE) {
        return 0;
    }
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !building_is_workshop(b->type)) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (b->subtype.workshop_type == output_type && b->road_network_id == road_network_id && b->loads_stored < 2) {
            int dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            if (b->loads_stored > 0) {
                dist += 20;
            }
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        *x_dst = min_building->road_access_x;
        *y_dst = min_building->road_access_y;
        return min_building->id;
    }
    return 0;
}

int building_get_workshop_for_raw_material(int x, int y, int resource,
                                           int distance_from_entry, int road_network_id,
                                           int *x_dst, int *y_dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE) {
        return 0;
    }
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !building_is_workshop(b->type)) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (b->subtype.workshop_type == output_type && b->road_network_id == road_network_id) {
            int dist = 10 * b->loads_stored +
                calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        *x_dst = min_building->road_access_x;
        *y_dst = min_building->road_access_y;
        return min_building->id;
    }
    return 0;
}
