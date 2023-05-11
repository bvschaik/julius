#include "industry.h"

#include "building/count.h"
#include "building/image.h"
#include "building/list.h"
#include "building/model.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/data_private.h"
#include "city/warning.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/random.h"
#include "figure/figure.h"
#include "game/time.h"
#include "map/building_tiles.h"
#include "scenario/property.h"

#define MAX_PROGRESS_VENUS_GT 400
#define DENARII_MINTED_PER_PRODUCTION 100
#define DENARII_COST_PER_GOLD 600
#define MAX_STORAGE 16
#define INFINITE 10000

#define RECORD_PRODUCTION_MONTHS 12

#define MERCURY_BLESSING_LOADS 3

int building_is_farm(building_type type)
{
    return type >= BUILDING_WHEAT_FARM && type <= BUILDING_PIG_FARM;
}

int building_is_raw_resource_producer(building_type type)
{
    return resource_is_raw_material(resource_get_from_industry(type));
}

int building_is_workshop(building_type type)
{
    resource_type good = resource_get_from_industry(type);
    return good != RESOURCE_NONE && resource_get_raw_materials_for_good(good) != 0;
}

int building_get_efficiency(const building *b)
{
    if (b->state == BUILDING_STATE_MOTHBALLED) {
        return -1;
    }
    int resource = resource_produced_by_building_type(b->type);
    if (b->data.industry.age_months == 0 || !resource) {
        return -1;
    }
    int production_for_resource = resource_production_per_month(resource);

    int percentage = calc_percentage(b->data.industry.average_production_per_month, production_for_resource);
    return calc_bound(percentage, 0, 100);
}

int building_industry_get_max_progress(const building *b)
{
    int monthly_production = resource_production_per_month(b->output_resource_id);
    return calc_percentage(GAME_TIME_DAYS_PER_MONTH * 2 * model_get_building(b->type)->laborers, monthly_production);
}

static void update_farm_image(const building *b)
{
    map_building_tiles_add_farm(b->id, b->x, b->y, building_image_get_base_farm_crop(b->type),
        calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b)));
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
    for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
        building_type type = resource_get_data(r)->industry;
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

static void update_venus_gt_production(void)
{
    building *venus_gt = building_first_of_type(BUILDING_GRAND_TEMPLE_VENUS);
    if (!venus_gt || !building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
        return;
    }
    
    venus_gt->data.monument.progress += (10 + (city_data.culture.population_with_venus_access /
        MAX_PROGRESS_VENUS_GT / 2));
    if (venus_gt->data.monument.progress > MAX_PROGRESS_VENUS_GT) {
        if (venus_gt->loads_stored < MAX_STORAGE) {
            venus_gt->loads_stored += 1;
        }
        venus_gt->data.monument.progress = venus_gt->data.monument.progress - MAX_PROGRESS_VENUS_GT;
    }
}

static void update_city_mint_production(int new_day)
{
    if (building_count_active(BUILDING_SENATE) == 0 && building_count_active(BUILDING_SENATE_UPGRADED) == 0) {
        return;
    }

    building *b = building_first_of_type(BUILDING_CITY_MINT);
    if (!b || b->state != BUILDING_STATE_IN_USE) {
        return;
    }

    b->data.industry.has_raw_materials = 0;
    if (b->houses_covered <= 0 || b->num_workers <= 0) {
        return;
    }

    if (b->loads_stored < BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN && b->output_resource_id == RESOURCE_DENARII) {
        return;
    }

    if (b->data.industry.curse_days_left && new_day) {
        b->data.industry.curse_days_left--;
        return;
    }

    if (b->output_resource_id == RESOURCE_GOLD) {
        if (b->data.industry.progress == 0) {
            if (city_finance_out_of_money()) {
                return;
            }
            city_finance_process_sundry(DENARII_COST_PER_GOLD);
        }
        b->data.industry.progress += b->num_workers;

        int max = building_industry_get_max_progress(b);
        if (b->data.industry.progress > max) {
            b->data.industry.progress = max;
        }
        return;
    }

    b->data.industry.progress += b->num_workers;

    int max = building_industry_get_max_progress(b);
    if (b->data.industry.progress > max) {
        b->data.industry.production_current_month += 100;
        b->data.industry.progress = 0;
        int minted_personal_funds = 0;
        city_finance_treasury_add_miscellaneous(DENARII_MINTED_PER_PRODUCTION);
        if (city_buildings_has_governor_house()) {
            int personal_salary = city_emperor_salary_amount();
            if (personal_salary > 5) {
                minted_personal_funds = DENARII_MINTED_PER_PRODUCTION / 10;
                minted_personal_funds = calc_adjust_with_percentage(minted_personal_funds, personal_salary);
                if (minted_personal_funds == 0) {
                    minted_personal_funds = 1;
                }
            }
        }
        city_data.emperor.personal_savings += minted_personal_funds;
        city_finance_treasury_add_miscellaneous(DENARII_MINTED_PER_PRODUCTION - minted_personal_funds);
        if (b->loads_stored >= BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN) {
            b->loads_stored -= BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN;
            if (b->loads_stored >= BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN) {
                b->data.industry.has_raw_materials = 1;
            }
        }
    }
}

void building_industry_update_production(int new_day)
{
    int striking_buildings = 0;

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        building_type type = resource_get_data(r)->industry;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE) {
                continue;
            }

            if (b->strike_duration_days > 0) {
                striking_buildings++;
                if (new_day) {
                    b->strike_duration_days--;
                    if (city_data.sentiment.value > 50) {
                        b->strike_duration_days -= 3;
                    }
                    if (city_data.sentiment.value > 65) {
                        b->strike_duration_days = 0;
                    }
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

            if (building_is_workshop(b->type) && !b->loads_stored) {
                continue;
            }

            if (b->data.industry.curse_days_left) {
                if (new_day) {
                    b->data.industry.curse_days_left--;
                }
                continue;
            }
            if (b->data.industry.blessing_days_left && new_day) {
                b->data.industry.blessing_days_left--;
            }
            b->data.industry.progress += b->num_workers;
            if (b->data.industry.blessing_days_left && building_is_farm(b->type)) {
                b->data.industry.progress += b->num_workers;
            }

            int max = building_industry_get_max_progress(b);
            if (b->data.industry.progress > max) {
                b->data.industry.progress = max;
            }
            if (building_is_farm(b->type)) {
                update_farm_image(b);
            }
        }
    }

    update_city_mint_production(new_day);

    if (new_day) {
        update_venus_gt_production();
        int num_strikes = city_data.building.num_striking_industries - striking_buildings;
        force_strike(num_strikes);
    }
}

int building_stockpiling_enabled(building *b)
{
    return b->data.industry.is_stockpiling;
}

int building_industry_has_produced_resource(building *b)
{
    if (b->type == BUILDING_CITY_MINT) {
        if (b->output_resource_id != RESOURCE_GOLD) {
            return 0;
        }
        if (b->loads_stored >= RESOURCE_ONE_LOAD) {
            return 1;
        }
    }
    return b->data.industry.progress >= building_industry_get_max_progress(b);
}

void building_industry_start_new_production(building *b)
{
    if (b->data.industry.progress >= building_industry_get_max_progress(b)) {
        b->data.industry.production_current_month += 100;
        b->data.industry.progress = 0;
    }
    int loads_when_complete = b->type == BUILDING_CITY_MINT ? RESOURCE_ONE_LOAD : 1;
    if (building_is_workshop(b->type) && b->loads_stored >= loads_when_complete) {
        if (b->loads_stored > loads_when_complete) {
            b->data.industry.has_raw_materials = 1;
        }
        b->loads_stored -= loads_when_complete;
    }
    if (building_is_farm(b->type)) {
        update_farm_image(b);
    }
}

void building_bless_farms(void)
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        building_type type = resource_get_data(r)->industry;
        if (!building_is_farm(type)) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE) {
                b->data.industry.progress = building_industry_get_max_progress(b);
                b->data.industry.curse_days_left = 0;
                b->data.industry.blessing_days_left = 16;
                update_farm_image(b);
            }
        }
    }
}

void building_bless_industry(void)
{
    for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
        building_type type = resource_get_data(r)->industry;
        if (!building_is_workshop(type)) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->loads_stored) {
                if (b->loads_stored < MERCURY_BLESSING_LOADS) {
                    b->loads_stored = MERCURY_BLESSING_LOADS;
                }
                b->data.industry.progress = building_industry_get_max_progress(b);
            }
        }
    }
    building *city_mint = building_first_of_type(BUILDING_CITY_MINT);
    if (city_mint && city_mint->state == BUILDING_STATE_IN_USE && city_mint->loads_stored > 0) {
        if (city_mint->loads_stored < MERCURY_BLESSING_LOADS * RESOURCE_ONE_LOAD) {
            city_mint->loads_stored = MERCURY_BLESSING_LOADS * RESOURCE_ONE_LOAD;
        }
        city_mint->data.industry.progress = building_industry_get_max_progress(city_mint);
    }
}

void building_curse_farms(int big_curse)
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        building_type type = resource_get_data(r)->industry;
        if (!building_is_farm(type)) {
            continue;
        }
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE) {
                b->data.industry.progress = 0;
                b->data.industry.blessing_days_left = 0;
                b->data.industry.curse_days_left = big_curse ? 48 : 4;
                update_farm_image(b);
            }
        }
    }
    building *city_mint = building_first_of_type(BUILDING_CITY_MINT);
    if (city_mint && city_mint->state == BUILDING_STATE_IN_USE) {
        city_mint->data.industry.progress = 0;
        city_mint->data.industry.curse_days_left = big_curse ? 48 : 4;
    }
}

static int is_valid_raw_material_for_building(building_type type, resource_type raw_material)
{
    resource_type good = resource_get_from_industry(type);
    if (good == RESOURCE_NONE) {
        return 0;
    }
    const resource_type *raw_materials = resource_get_raw_materials_for_good(good);
    for (int i = 0; raw_materials[i] != RESOURCE_NONE; i++) {
        if (raw_materials[i] == raw_material) {
            return 1;
        }
    }
    return 0;
}

void building_workshop_add_raw_material(building *b, int resource)
{
    if (!b->id) {
        return;
    }
    if (b->type == BUILDING_CITY_MINT && resource == RESOURCE_GOLD) {
        b->loads_stored += RESOURCE_ONE_LOAD;
    } else if (building_is_workshop(b->type) && is_valid_raw_material_for_building(b->type, resource)) {
        b->loads_stored++;
    }
}

int building_has_workshop_for_raw_material_with_room(int resource, int road_network_id)
{
    const resource_type *goods = resource_get_goods_from_raw_material(resource);

    if (goods == 0) {
        return 0;
    }
    for (int i = 0; goods[i] != RESOURCE_NONE; i++) {
        building_type type = resource_get_data(goods[i])->industry;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->has_road_access && b->distance_from_entry > 0 &&
                b->road_network_id == road_network_id && b->loads_stored < 2) {
                return 1;
            }
        }
    }
    return 0;
}

int building_get_workshop_for_raw_material_with_room(int x, int y, int resource, int road_network_id, map_point *dst)
{
    if (city_resource_is_stockpiled(resource)) {
        return 0;
    }
    const resource_type *goods = resource_get_goods_from_raw_material(resource);

    if (goods == 0) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    for (int i = 0; goods[i] != RESOURCE_NONE; i++) {
        building_type type = resource_get_data(goods[i])->industry;
        int max_loads_stored = type == BUILDING_CITY_MINT ? 2 * RESOURCE_ONE_LOAD : 2;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->has_road_access || b->distance_from_entry <= 0 ||
                b->road_network_id != road_network_id || b->loads_stored >= max_loads_stored) {
                continue;
            }
            if (type == BUILDING_CITY_MINT) {
                if (b->data.monument.phase != MONUMENT_FINISHED || b->output_resource_id == RESOURCE_GOLD) {
                    continue;
                }
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
    const resource_type *goods = resource_get_goods_from_raw_material(resource);

    if (goods == 0) {
        return 0;
    }
    int min_dist = INFINITE;
    building *min_building = 0;
    for (int i = 0; goods[i] != RESOURCE_NONE; i++) {
        building_type type = resource_get_data(goods[i])->industry;
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE ||
                !b->has_road_access || b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
                continue;
            }
            if (type == BUILDING_CITY_MINT) {
                if (b->data.monument.phase != MONUMENT_FINISHED || b->output_resource_id == RESOURCE_GOLD) {
                    continue;
                }
            }
            int dist = 10 * b->loads_stored +
                calc_maximum_distance(b->x, b->y, x, y);
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    }
    return 0;
}

static void update_stats_for_type(building_type type)
{
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_MOTHBALLED) {
            continue;
        }
        if (b->data.industry.age_months < RECORD_PRODUCTION_MONTHS) {
            b->data.industry.age_months++;
        }
        int sum_months = b->data.industry.average_production_per_month * (b->data.industry.age_months - 1);
        int pending_production_percentage = b->type == BUILDING_WHARF ?
            0 : calc_percentage(b->data.industry.progress, building_industry_get_max_progress(b));
        pending_production_percentage = calc_bound(pending_production_percentage, 0, 100);
        sum_months += b->data.industry.production_current_month + pending_production_percentage;
        b->data.industry.average_production_per_month = sum_months / b->data.industry.age_months;
        int leftover_from_average = sum_months % b->data.industry.age_months;
        b->data.industry.production_current_month = leftover_from_average - pending_production_percentage;
    }
}

void building_industry_advance_stats(void)
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        update_stats_for_type(resource_get_data(r)->industry);
    }
    update_stats_for_type(BUILDING_CITY_MINT);
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

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        building_type type = resource_get_data(r)->industry;
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
