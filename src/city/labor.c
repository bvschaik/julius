#include "labor.h"

#include "building/building.h"
#include "building/model.h"
#include "core/config.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/time.h"
#include "scenario/property.h"

#define MAX_CATS 10

typedef enum {
    LABOR_CATEGORY_INDUSTRY_COMMERCE = 0,
    LABOR_CATEGORY_FOOD_PRODUCTION = 1,
    LABOR_CATEGORY_ENGINEERING = 2,
    LABOR_CATEGORY_WATER = 3,
    LABOR_CATEGORY_PREFECTURES = 4,
    LABOR_CATEGORY_MILITARY = 5,
    LABOR_CATEGORY_ENTERTAINMENT = 6,
    LABOR_CATEGORY_HEALTH_EDUCATION = 7,
    LABOR_CATEGORY_GOVERNANCE_RELIGION = 8
} labor_category;

static int CATEGORY_FOR_BUILDING_TYPE[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 20
    6, 6, 6, 6, 6, 6, 6, 6, -1, -1, // 30
    -1, -1, -1, -1, -1, -1, 7, 7, 7, 7, // 40
    0, 7, 7, 7, -1, 4, -1, 5, 5, 5, // 50
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, // 60
    0, 1, 0, -1, 1, 0, 1, -1, -1, -1, // 70
    7, 2, -1, -1, 8, 8, 8, 8, -1, -1, // 80
    -1, 3, -1, -1, 5, 5, -1, -1, 8, -1, // 90
    1, 1, 1, 0, 0, 1, 0, 0, 0, 0, // 100
    0, 0, 0, 0, 0, -1, -1, -1, -1, -1, // 110
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 //120
};

static struct {
    labor_category category;
    int workers;
} DEFAULT_PRIORITY[MAX_CATS] = {
    {LABOR_CATEGORY_ENGINEERING, 3},
    {LABOR_CATEGORY_WATER, 1},
    {LABOR_CATEGORY_PREFECTURES, 3},
    {LABOR_CATEGORY_MILITARY, 2},
    {LABOR_CATEGORY_FOOD_PRODUCTION, 4},
    {LABOR_CATEGORY_INDUSTRY_COMMERCE, 2},
    {LABOR_CATEGORY_ENTERTAINMENT, 1},
    {LABOR_CATEGORY_HEALTH_EDUCATION, 1},
    {LABOR_CATEGORY_GOVERNANCE_RELIGION, 1},
};

int city_labor_unemployment_percentage(void)
{
    return city_data.labor.unemployment_percentage;
}

int city_labor_unemployment_percentage_for_senate(void)
{
    return city_data.labor.unemployment_percentage_for_senate;
}

int city_labor_workers_needed(void)
{
    return city_data.labor.workers_needed;
}

int city_labor_workers_employed(void)
{
    return city_data.labor.workers_employed;
}

int city_labor_workers_unemployed(void)
{
    return city_data.labor.workers_unemployed;
}

int city_labor_wages(void)
{
    return city_data.labor.wages;
}

void city_labor_change_wages(int amount)
{
    city_data.labor.wages += amount;
    city_data.labor.wages = calc_bound(city_data.labor.wages, 0, 100);
}

int city_labor_wages_rome(void)
{
    return city_data.labor.wages_rome;
}

int city_labor_raise_wages_rome(void)
{
    if (city_data.labor.wages_rome >= 45) {
        return 0;
    }
    city_data.labor.wages_rome += 1 + (random_byte_alt() & 3);
    if (city_data.labor.wages_rome > 45) {
        city_data.labor.wages_rome = 45;
    }
    return 1;
}

int city_labor_lower_wages_rome(void)
{
    if (city_data.labor.wages_rome <= 5) {
        return 0;
    }
    city_data.labor.wages_rome -= 1 + (random_byte_alt() & 3);
    return 1;
}

const labor_category_data *city_labor_category(int category)
{
    return &city_data.labor.categories[category];
}

void city_labor_calculate_workers(int num_plebs, int num_patricians)
{
    city_data.population.percentage_plebs = calc_percentage(num_plebs, num_plebs + num_patricians);

    if (config_get(CONFIG_GP_CH_FIXED_WORKERS)) {    
        city_data.population.working_age = calc_adjust_with_percentage(num_plebs, 38);
        city_data.labor.workers_available = city_data.population.working_age;
    } else {
        city_data.population.working_age = calc_adjust_with_percentage(city_population_people_of_working_age(), 60);
        city_data.labor.workers_available = calc_adjust_with_percentage(
            city_data.population.working_age, city_data.population.percentage_plebs);
    }
}

static int is_industry_disabled(building *b) {
    if (b->type < BUILDING_WHEAT_FARM || b->type > BUILDING_POTTERY_WORKSHOP) {
        return 0;
    }
    int resource = b->output_resource_id;
    if (city_data.resource.mothballed[resource]) {
        return 1;
    }
    return 0;
}

static int should_have_workers(building *b, int category, int check_access)
{
    if (category < 0) {
        return 0;
    }

    if (category == LABOR_CATEGORY_ENTERTAINMENT) {
        if (b->type == BUILDING_HIPPODROME && b->prev_part_building_id) {
            return 0;
        }
    } else if (category == LABOR_CATEGORY_FOOD_PRODUCTION || category == LABOR_CATEGORY_INDUSTRY_COMMERCE) {
        if (is_industry_disabled(b)) {
            return 0;
        }
    }
    // engineering and water are always covered
    if (category == LABOR_CATEGORY_ENGINEERING || category == LABOR_CATEGORY_WATER) {
        return 1;
    }
    if (check_access) {
        return b->houses_covered > 0 ? 1 : 0;
    }
    return 1;
}

static void calculate_workers_needed_per_category(void)
{
    for (int cat = 0; cat < MAX_CATS; cat++) {
        city_data.labor.categories[cat].buildings = 0;
        city_data.labor.categories[cat].total_houses_covered = 0;
        city_data.labor.categories[cat].workers_allocated = 0;
        city_data.labor.categories[cat].workers_needed = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int category = CATEGORY_FOR_BUILDING_TYPE[b->type];
        b->labor_category = category;
        if (!should_have_workers(b, category, 1)) {
            continue;
        }
        city_data.labor.categories[category].workers_needed += model_get_building(b->type)->laborers;
        city_data.labor.categories[category].total_houses_covered += b->houses_covered;
        city_data.labor.categories[category].buildings++;
    }
}

static void allocate_workers_to_categories(void)
{
    int workers_needed = 0;
    for (int i = 0; i < MAX_CATS; i++) {
        city_data.labor.categories[i].workers_allocated = 0;
        workers_needed += city_data.labor.categories[i].workers_needed;
    }
    city_data.labor.workers_needed = 0;
    if (workers_needed <= city_data.labor.workers_available) {
        for (int i = 0; i < MAX_CATS; i++) {
            city_data.labor.categories[i].workers_allocated = city_data.labor.categories[i].workers_needed;
        }
        city_data.labor.workers_employed = workers_needed;
    } else {
        // not enough workers
        int available = city_data.labor.workers_available;
        // distribute by user-defined priority
        for (int p = 1; p <= 9 && available > 0; p++) {
            for (int c = 0; c < 9; c++) {
                if (p == city_data.labor.categories[c].priority) {
                    int to_allocate = city_data.labor.categories[c].workers_needed;
                    if (to_allocate > available) {
                        to_allocate = available;
                    }
                    city_data.labor.categories[c].workers_allocated = to_allocate;
                    available -= to_allocate;
                    break;
                }
            }
        }
        // (sort of) round-robin distribution over unprioritized categories:
        int guard = 0;
        do {
            guard++;
            if (guard >= city_data.labor.workers_available) {
                break;
            }
            for (int p = 0; p < 9; p++) {
                int cat = DEFAULT_PRIORITY[p].category;
                if (!city_data.labor.categories[cat].priority) {
                    int needed = city_data.labor.categories[cat].workers_needed - city_data.labor.categories[cat].workers_allocated;
                    if (needed > 0) {
                        int to_allocate = DEFAULT_PRIORITY[p].workers;
                        if (to_allocate > available) {
                            to_allocate = available;
                        }
                        if (to_allocate > needed) {
                            to_allocate = needed;
                        }
                        city_data.labor.categories[cat].workers_allocated += to_allocate;
                        available -= to_allocate;
                        if (available <= 0) {
                            break;
                        }
                    }
                }
            }
        } while (available > 0);

        city_data.labor.workers_employed = city_data.labor.workers_available;
        for (int i = 0; i < 9; i++) {
            city_data.labor.workers_needed +=
                city_data.labor.categories[i].workers_needed - city_data.labor.categories[i].workers_allocated;
        }
    }
    city_data.labor.workers_unemployed = city_data.labor.workers_available - city_data.labor.workers_employed;
    city_data.labor.unemployment_percentage =
        calc_percentage(city_data.labor.workers_unemployed, city_data.labor.workers_available);
}

static void check_employment(void)
{
    int orig_needed = city_data.labor.workers_needed;
    allocate_workers_to_categories();
    // senate unemployment display is delayed when unemployment is rising
    if (city_data.labor.unemployment_percentage < city_data.labor.unemployment_percentage_for_senate) {
        city_data.labor.unemployment_percentage_for_senate = city_data.labor.unemployment_percentage;
    } else if (city_data.labor.unemployment_percentage < city_data.labor.unemployment_percentage_for_senate + 5) {
        city_data.labor.unemployment_percentage_for_senate = city_data.labor.unemployment_percentage;
    } else {
        city_data.labor.unemployment_percentage_for_senate += 5;
    }
    if (city_data.labor.unemployment_percentage_for_senate > 100) {
        city_data.labor.unemployment_percentage_for_senate = 100;
    }

    // workers needed message
    if (!orig_needed && city_data.labor.workers_needed > 0) {
        if (game_time_year() >= scenario_property_start_year()) {
            city_message_post_with_message_delay(MESSAGE_CAT_WORKERS_NEEDED, 0, MESSAGE_WORKERS_NEEDED, 6);
        }
    }
}

static void set_building_worker_weight(void)
{
    int water_per_10k_per_building = calc_percentage(100, city_data.labor.categories[LABOR_CATEGORY_WATER].buildings);
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int cat = CATEGORY_FOR_BUILDING_TYPE[b->type];
        if (cat == LABOR_CATEGORY_WATER) {
            b->percentage_houses_covered = water_per_10k_per_building;
        } else if (cat >= 0) {
            b->percentage_houses_covered = 0;
            if (b->houses_covered) {
                b->percentage_houses_covered =
                    calc_percentage(100 * b->houses_covered,
                        city_data.labor.categories[cat].total_houses_covered);
            }
        }
    }
}

static void allocate_workers_to_water(void)
{
    static int start_building_id = 1;
    labor_category_data *water_cat = &city_data.labor.categories[LABOR_CATEGORY_WATER];

    int percentage_not_filled = 100 - calc_percentage(water_cat->workers_allocated, water_cat->workers_needed);

    int buildings_to_skip = calc_adjust_with_percentage(water_cat->buildings, percentage_not_filled);

    int workers_per_building;
    if (buildings_to_skip == water_cat->buildings) {
        workers_per_building = 1;
    } else {
        workers_per_building = water_cat->workers_allocated / (water_cat->buildings - buildings_to_skip);
    }
    int building_id = start_building_id;
    start_building_id = 0;
    for (int guard = 1; guard < MAX_BUILDINGS; guard++, building_id++) {
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (b->state != BUILDING_STATE_IN_USE || CATEGORY_FOR_BUILDING_TYPE[b->type] != LABOR_CATEGORY_WATER) {
            continue;
        }
        b->num_workers = 0;
        if (b->percentage_houses_covered > 0) {
            if (percentage_not_filled > 0) {
                if (buildings_to_skip) {
                    --buildings_to_skip;
                } else if (start_building_id) {
                    b->num_workers = workers_per_building;
                } else {
                    start_building_id = building_id;
                    b->num_workers = workers_per_building;
                }
            } else {
                b->num_workers = model_get_building(b->type)->laborers;
            }
        }
    }
    if (!start_building_id) {
        // no buildings assigned or full employment
        start_building_id = 1;
    }
}

static void allocate_workers_to_non_water_buildings(void)
{
    int category_workers_needed[MAX_CATS];
    int category_workers_allocated[MAX_CATS];
    for (int i = 0; i < MAX_CATS; i++) {
        category_workers_allocated[i] = 0;
        category_workers_needed[i] =
            city_data.labor.categories[i].workers_allocated < city_data.labor.categories[i].workers_needed
            ? 1 : 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int cat = CATEGORY_FOR_BUILDING_TYPE[b->type];
        if (cat == LABOR_CATEGORY_WATER || cat < 0) {
            // water is handled by allocate_workers_to_water(void)
            continue;
        }
        b->num_workers = 0;
        if (!should_have_workers(b, cat, 0)) {
            continue;
        }
        if (b->percentage_houses_covered > 0) {
            int required_workers = model_get_building(b->type)->laborers;
            if (category_workers_needed[cat]) {
                int num_workers = calc_adjust_with_percentage(
                    city_data.labor.categories[cat].workers_allocated,
                    b->percentage_houses_covered) / 100;
                if (num_workers > required_workers) {
                    num_workers = required_workers;
                }
                b->num_workers = num_workers;
                category_workers_allocated[cat] += num_workers;
            } else {
                b->num_workers = required_workers;
            }
        }
    }
    for (int i = 0; i < MAX_CATS; i++) {
        if (category_workers_needed[i]) {
            // watch out: category_workers_needed is now reset to 'unallocated workers available'
            if (category_workers_allocated[i] >= city_data.labor.categories[i].workers_allocated) {
                category_workers_needed[i] = 0;
                category_workers_allocated[i] = 0;
            } else {
                category_workers_needed[i] = city_data.labor.categories[i].workers_allocated - category_workers_allocated[i];
            }
        }
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int cat = CATEGORY_FOR_BUILDING_TYPE[b->type];
        if (cat < 0 || cat == LABOR_CATEGORY_WATER || cat == LABOR_CATEGORY_MILITARY) {
            continue;
        }
        if (!should_have_workers(b, cat, 0)) {
            continue;
        }
        if (b->percentage_houses_covered > 0 && category_workers_needed[cat]) {
            int required_workers = model_get_building(b->type)->laborers;
            if (b->num_workers < required_workers) {
                int needed = required_workers - b->num_workers;
                if (needed > category_workers_needed[cat]) {
                    b->num_workers += category_workers_needed[cat];
                    category_workers_needed[cat] = 0;
                } else {
                    b->num_workers += needed;
                    category_workers_needed[cat] -= needed;
                }
            }
        }
    }
}

static void allocate_workers_to_buildings(void)
{
    set_building_worker_weight();
    allocate_workers_to_water();
    allocate_workers_to_non_water_buildings();
}

void city_labor_allocate_workers(void)
{
    allocate_workers_to_categories();
    allocate_workers_to_buildings();
}

void city_labor_update(void)
{
    calculate_workers_needed_per_category();
    check_employment();
    allocate_workers_to_buildings();
}

void city_labor_set_priority(int category, int new_priority)
{
    int old_priority = city_data.labor.categories[category].priority;
    if (old_priority == new_priority) {
        return;
    }
    int shift;
    int from_prio;
    int to_prio;
    if (!old_priority && new_priority) {
        // shift all bigger than 'new_priority' by one down (+1)
        shift = 1;
        from_prio = new_priority;
        to_prio = 9;
    } else if (old_priority && !new_priority) {
        // shift all bigger than 'old_priority' by one up (-1)
        shift = -1;
        from_prio = old_priority;
        to_prio = 9;
    } else if (new_priority < old_priority) {
        // shift all between new and old by one down (+1)
        shift = 1;
        from_prio = new_priority;
        to_prio = old_priority;
    } else {
        // shift all between old and new by one up (-1)
        shift = -1;
        from_prio = old_priority;
        to_prio = new_priority;
    }
    city_data.labor.categories[category].priority = new_priority;
    for (int i = 0; i < 9; i++) {
        if (i == category) {
            continue;
        }
        int current_priority = city_data.labor.categories[i].priority;
        if (from_prio <= current_priority && current_priority <= to_prio) {
            city_data.labor.categories[i].priority += shift;
        }
    }
    city_labor_allocate_workers();
}

int city_labor_max_selectable_priority(int category)
{
    int max = 0;
    for (int i = 0; i < 9; i++) {
        if (city_data.labor.categories[i].priority > 0) {
            ++max;
        }
    }
    if (max < 9 && !city_data.labor.categories[category].priority) {
        // allow space for new priority
        ++max;
    }
    return max;
}
