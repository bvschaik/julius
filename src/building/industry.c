#include "industry.h"

#include "building/list.h"
#include "building/monument.h"
#include "city/data_private.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/figure.h"
#include "map/building_tiles.h"
#include "scenario/property.h"

#define MAX_PROGRESS_RAW 200
#define MAX_PROGRESS_WORKSHOP 400
#define MAX_STORAGE 16
#define INFINITE 10000

#define RECORD_PRODUCTION_MONTHS 12

#define MERCURY_BLESSING_LOADS 3

enum {
    MIN_FARM = 0,
    MIN_FOOD_PRODUCER = 0,
    MAX_FARM = 5,
    MAX_FOOD_PRODUCER = 6,
    MIN_RAW_MATERIALS = 7,
    MAX_RAW_MATERIALS = 10,
    MIN_WORKSHOP = 11,
    MAX_WORKSHOP = 15,
    VENUS_TEMPLE = 16,
    MAX_INDUSTRY_TYPES = 17
};

static const building_type INDUSTRY_TYPES[] = {
    BUILDING_WHEAT_FARM,
    BUILDING_VEGETABLE_FARM,
    BUILDING_FRUIT_FARM,
    BUILDING_OLIVE_FARM,
    BUILDING_VINES_FARM,
    BUILDING_PIG_FARM,
    BUILDING_WHARF,
    BUILDING_MARBLE_QUARRY,
    BUILDING_IRON_MINE,
    BUILDING_TIMBER_YARD,
    BUILDING_CLAY_PIT,
    BUILDING_WINE_WORKSHOP,
    BUILDING_OIL_WORKSHOP,
    BUILDING_WEAPONS_WORKSHOP,
    BUILDING_FURNITURE_WORKSHOP,
    BUILDING_POTTERY_WORKSHOP,
    BUILDING_GRAND_TEMPLE_VENUS,
};

static const building_type OUTPUT_TYPE_TO_INDUSTRY[] = {
    BUILDING_NONE,
    BUILDING_OIL_WORKSHOP,
    BUILDING_WINE_WORKSHOP,
    BUILDING_WEAPONS_WORKSHOP,
    BUILDING_FURNITURE_WORKSHOP,
    BUILDING_POTTERY_WORKSHOP
};

int building_is_farm(building_type type)
{
    return type >= BUILDING_WHEAT_FARM && type <= BUILDING_PIG_FARM;
}

int building_is_raw_resource_producer(building_type type)
{
    return type >= BUILDING_MARBLE_QUARRY && type <= BUILDING_CLAY_PIT;
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

static void building_other_update_production(building *b)
{
    // For monuments or other non-industry subtypes that generate goods
    if (building_monument_is_monument(b)) {
        b->data.monument.progress += (10 + (city_data.culture.population_with_venus_access / 200));
        if (b->data.monument.progress > MAX_PROGRESS_WORKSHOP) {
            if (b->loads_stored < MAX_STORAGE) {
                b->loads_stored += 1;
            }
            b->data.monument.progress = b->data.monument.progress - MAX_PROGRESS_WORKSHOP;
        }
    }
}

static int random_industry_strikes(int num_strikes)
{
    int strikes = 0;
    int total_industries = building_list_large_size();
    if (num_strikes >= total_industries) {
        for (int i = 0; i < total_industries; i++) {
            building *b = building_get(building_list_large_item(i));
            if (b->strike_duration_days == 0) {
                b->strike_duration_days = 48;
                strikes++;
            }
        }
        return strikes;
    }

    for (int i = 0; i < num_strikes; i++) {
        int index = random_from_stdlib() % total_industries;

        // Prevent the same building from being selected twice
        int current = index + 1;
        building *b = building_get(building_list_large_item(index));
        while (b->strike_duration_days > 0) {
            if (current == total_industries) {
                current = 0;
            }
            b = building_get(building_list_large_item(current));
            current++;
            if (current == index) {
                return strikes;
            }
        }
        b->strike_duration_days = 48;
        strikes++;
    }
    return strikes;
}

static void force_strike(int num_strikes)
{
    building_list_large_clear();
    for (int i = MIN_RAW_MATERIALS; i <= MAX_WORKSHOP; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->strike_duration_days == 0) {
                building_list_large_add(b->id);
            }
        }
    }
    if (random_industry_strikes(num_strikes)) {
        city_warning_show(WARNING_SECESSION, NEW_WARNING_SLOT);
    }
}

void building_industry_update_production(void)
{
    int striking_buildings = 0;

    for (int i = 0; i < MAX_INDUSTRY_TYPES; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE) {
                continue;
            }

            if (b->strike_duration_days > 0) {
                striking_buildings++;
                b->strike_duration_days--;
                if (city_data.sentiment.value > 50) {
                    b->strike_duration_days -= 3;
                }
                if (city_data.sentiment.value > 65) {
                    b->strike_duration_days = 0;
                }
                if (b->strike_duration_days == 0) {
                    city_data.building.num_striking_industries--;
                    striking_buildings--;
                    // remove striker walker
                    figure_delete(figure_get(b->figure_id4));
                }
            }

            b->data.industry.has_raw_materials = 0;
            if (b->houses_covered <= 0 || b->num_workers <= 0 || b->strike_duration_days > 0) {
                continue;
            }

            if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE) &&
                b->type == BUILDING_GRAND_TEMPLE_VENUS) {
                building_other_update_production(b);
                continue;
            }

            if (b->subtype.workshop_type && !b->loads_stored) {
                continue;
            }

            if (b->data.industry.curse_days_left) {
                b->data.industry.curse_days_left--;
                continue;
            }
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

    int num_strikes = city_data.building.num_striking_industries - striking_buildings;
    force_strike(num_strikes);
}

void building_industry_update_wheat_production(void)
{
    if (scenario_property_climate() == CLIMATE_NORTHERN) {
        return;
    }
    for (building *b = building_first_of_type(BUILDING_WHEAT_FARM); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || b->houses_covered <= 0 ||
            b->num_workers <= 0 || b->data.industry.curse_days_left) {
            continue;
        }
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

int building_stockpiling_enabled(building *b)
{
    return b->data.industry.is_stockpiling;
}

int building_industry_has_produced_resource(building *b)
{
    return b->data.industry.progress >= max_progress(b);
}

static void update_production_stats(building *b)
{
    b->data.industry.production_current_month += 100;
}

void building_industry_start_new_production(building *b)
{
    update_production_stats(b);
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
    for (int i = MIN_FARM; i <= MAX_FARM; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE) {
                b->data.industry.progress = MAX_PROGRESS_RAW;
                b->data.industry.curse_days_left = 0;
                b->data.industry.blessing_days_left = 16;
                update_farm_image(b);
            }
        }
    }
}

void building_bless_industry(void)
{
    for (int i = MIN_WORKSHOP; i <= MAX_WORKSHOP; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->loads_stored) {
                if (b->loads_stored < MERCURY_BLESSING_LOADS) {
                    b->loads_stored = MERCURY_BLESSING_LOADS;
                }
                b->data.industry.progress = MAX_PROGRESS_WORKSHOP;
            }
        }
    }
}

void building_curse_farms(int big_curse)
{
    for (int i = MIN_FARM; i <= MAX_FARM; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE) {
                b->data.industry.progress = 0;
                b->data.industry.blessing_days_left = 0;
                b->data.industry.curse_days_left = big_curse ? 48 : 4;
                update_farm_image(b);
            }
        }
    }
}

void building_workshop_add_raw_material(building *b)
{
    if (b->id > 0 && building_is_workshop(b->type)) {
        b->loads_stored++; // BUG: any raw material accepted
    }
}

int building_has_workshop_for_raw_material_with_room(int workshop_type, int road_network_id)
{
    building_type type = OUTPUT_TYPE_TO_INDUSTRY[workshop_type];
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE && b->has_road_access && b->distance_from_entry > 0 &&
            b->road_network_id == road_network_id && b->loads_stored < 2) {
            return 1;
        }
    }
    return 0;
}

int building_get_workshop_for_raw_material_with_room(int x, int y,
    int resource, int road_network_id, map_point *dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    building_type type = OUTPUT_TYPE_TO_INDUSTRY[output_type];
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE || !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != road_network_id || b->loads_stored >= 2) {
            continue;
        }
        int dist = calc_maximum_distance(b->x, b->y, x, y);
        if (b->loads_stored > 0) {
            dist += 20;
        }
        if (dist < min_dist) {
            min_dist = dist;
            min_building = b;
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    }
    return 0;
}

int building_get_workshop_for_raw_material(int x, int y, int resource, int road_network_id, map_point *dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    int output_type = resource_to_workshop_type(resource);
    if (output_type == WORKSHOP_NONE) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    building_type type = OUTPUT_TYPE_TO_INDUSTRY[output_type];
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE ||
            !b->has_road_access || b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
            continue;
        }
        int dist = 10 * b->loads_stored +
            calc_maximum_distance(b->x, b->y, x, y);
        if (dist < min_dist) {
            min_dist = dist;
            min_building = b;
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    }
    return 0;
}

void building_industry_advance_stats(void)
{
    for (int i = MIN_FARM; i <= MAX_WORKSHOP; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_MOTHBALLED) {
                continue;
            }
            if (b->data.industry.age_months < RECORD_PRODUCTION_MONTHS) {
                b->data.industry.age_months++;
            }
            int sum_months = b->data.industry.average_production_per_month * (b->data.industry.age_months - 1);
            int pending_production_percentage = b->type == BUILDING_WHARF ?
                0 : calc_percentage(b->data.industry.progress, max_progress(b));
            pending_production_percentage = calc_bound(pending_production_percentage, 0, 100);
            sum_months += b->data.industry.production_current_month + pending_production_percentage;
            b->data.industry.average_production_per_month = sum_months / b->data.industry.age_months;
            int leftover_from_average = sum_months % b->data.industry.age_months;
            b->data.industry.production_current_month = leftover_from_average - pending_production_percentage;
        }
    }
}

void building_industry_start_strikes(void)
{
    if (city_data.sentiment.value >= 55) {
        return;
    }
    int base_chance = 60 - city_data.sentiment.value;
    int strike_chance = base_chance * base_chance / 36;
    if (random_from_stdlib() % 100 > strike_chance) {
        return;
    }
    int to_strike = calc_bound(city_data.population.population / 2000, 1, 12);

    building_list_large_clear();

    for (int i = MIN_RAW_MATERIALS; i <= MAX_WORKSHOP; i++) {
        building_type type = INDUSTRY_TYPES[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->strike_duration_days == 0) {
                building_list_large_add(b->id);
            }
        }
    }

    int strikes = random_industry_strikes(to_strike);

    city_data.building.num_striking_industries += strikes;

    if (strikes) {
        city_warning_show(WARNING_SECESSION, NEW_WARNING_SLOT);
    }
}
