#include "labor.h"

#include "building/building.h"
#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "game/time.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

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
    0, 1, 0, -1, 0, 0, 0, -1, -1, -1, // 70
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

// static void setBuildingWorkerWeight();
// static void allocateWorkersToWater();
// static void allocateWorkersToBuildings();

static int is_industry_disabled(building *b) {
    if (b->type < BUILDING_WHEAT_FARM || b->type > BUILDING_POTTERY_WORKSHOP) {
        return 0;
    }
    int resource = b->outputResourceId;
    if (Data_CityInfo.resourceIndustryMothballed[resource]) {
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
        if (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId) {
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
        return b->housesCovered > 0 ? 1 : 0;
    }
    return 1;
}

static void calculate_workers_needed_per_category()
{
    for (int cat = 0; cat < MAX_CATS; cat++) {
        Data_CityInfo.laborCategory[cat].buildings = 0;
        Data_CityInfo.laborCategory[cat].totalHousesCovered = 0;
        Data_CityInfo.laborCategory[cat].workersAllocated = 0;
        Data_CityInfo.laborCategory[cat].workersNeeded = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int category = CATEGORY_FOR_BUILDING_TYPE[b->type];
        b->laborCategory = category;
        if (!should_have_workers(b, category, 1)) {
            continue;
        }
        Data_CityInfo.laborCategory[category].workersNeeded += model_get_building(b->type)->laborers;
        Data_CityInfo.laborCategory[category].totalHousesCovered += b->housesCovered;
        Data_CityInfo.laborCategory[category].buildings++;
    }
}

static void allocate_workers_to_categories()
{
    int workers_needed = 0;
    for (int i = 0; i < MAX_CATS; i++) {
        Data_CityInfo.laborCategory[i].workersAllocated = 0;
        workers_needed += Data_CityInfo.laborCategory[i].workersNeeded;
    }
    Data_CityInfo.workersNeeded = 0;
    if (workers_needed <= Data_CityInfo.workersAvailable) {
        for (int i = 0; i < MAX_CATS; i++) {
            Data_CityInfo.laborCategory[i].workersAllocated = Data_CityInfo.laborCategory[i].workersNeeded;
        }
        Data_CityInfo.workersEmployed = workers_needed;
    } else {
        // not enough workers
        int available = Data_CityInfo.workersAvailable;
        // distribute by user-defined priority
        for (int p = 1; p <= 9 && available > 0; p++) {
            for (int c = 0; c < 9; c++) {
                if (p == Data_CityInfo.laborCategory[c].priority) {
                    int to_allocate = Data_CityInfo.laborCategory[c].workersNeeded;
                    if (to_allocate > available) {
                        to_allocate = available;
                    }
                    Data_CityInfo.laborCategory[c].workersAllocated = to_allocate;
                    available -= to_allocate;
                    break;
                }
            }
        }
        // (sort of) round-robin distribution over unprioritized categories:
        int guard = 0;
        do {
            guard++;
            if (guard >= Data_CityInfo.workersAvailable) {
                break;
            }
            for (int p = 0; p < 9; p++) {
                int cat = DEFAULT_PRIORITY[p].category;
                if (!Data_CityInfo.laborCategory[cat].priority) {
                    int needed = Data_CityInfo.laborCategory[cat].workersNeeded - Data_CityInfo.laborCategory[cat].workersAllocated;
                    if (needed > 0) {
                        int to_allocate = DEFAULT_PRIORITY[p].workers;
                        if (to_allocate > available) {
                            to_allocate = available;
                        }
                        if (to_allocate > needed) {
                            to_allocate = needed;
                        }
                        Data_CityInfo.laborCategory[cat].workersAllocated += to_allocate;
                        available -= to_allocate;
                        if (available <= 0) {
                            break;
                        }
                    }
                }
            }
        } while (available > 0);

        Data_CityInfo.workersEmployed = Data_CityInfo.workersAvailable;
        for (int i = 0; i < 9; i++) {
            Data_CityInfo.workersNeeded +=
                Data_CityInfo.laborCategory[i].workersNeeded - Data_CityInfo.laborCategory[i].workersAllocated;
        }
    }
    Data_CityInfo.workersUnemployed = Data_CityInfo.workersAvailable - Data_CityInfo.workersEmployed;
    Data_CityInfo.unemploymentPercentage =
        calc_percentage(Data_CityInfo.workersUnemployed, Data_CityInfo.workersAvailable);
}

static void check_employment()
{
    int orig_needed = Data_CityInfo.workersNeeded;
    allocate_workers_to_categories();
    // senate unemployment display is delayed when unemployment is rising
    if (Data_CityInfo.unemploymentPercentage < Data_CityInfo.unemploymentPercentageForSenate) {
        Data_CityInfo.unemploymentPercentageForSenate = Data_CityInfo.unemploymentPercentage;
    } else if (Data_CityInfo.unemploymentPercentage < Data_CityInfo.unemploymentPercentageForSenate + 5) {
        Data_CityInfo.unemploymentPercentageForSenate = Data_CityInfo.unemploymentPercentage;
    } else {
        Data_CityInfo.unemploymentPercentageForSenate += 5;
    }
    if (Data_CityInfo.unemploymentPercentageForSenate > 100) {
        Data_CityInfo.unemploymentPercentageForSenate = 100;
    }

    // workers needed message
    if (!orig_needed && Data_CityInfo.workersNeeded > 0) {
        if (game_time_year() >= scenario_property_start_year()) {
            city_message_post_with_message_delay(MESSAGE_CAT_WORKERS_NEEDED, 0, MESSAGE_WORKERS_NEEDED, 6);
        }
    }
}

static void set_building_worker_weight()
{
    int waterPer10kPerBuilding = calc_percentage(100, Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].buildings);
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int cat = CATEGORY_FOR_BUILDING_TYPE[b->type];
        if (cat == LABOR_CATEGORY_WATER) {
            b->percentageHousesCovered = waterPer10kPerBuilding;
        } else if (cat >= 0) {
            b->percentageHousesCovered = 0;
            if (b->housesCovered) {
                b->percentageHousesCovered =
                    calc_percentage(100 * b->housesCovered,
                        Data_CityInfo.laborCategory[cat].totalHousesCovered);
            }
        }
    }
}

static void allocate_workers_to_water()
{
    static int start_building_id = 1;

    int percentage_not_filled = 100 - calc_percentage(
        Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].workersAllocated,
        Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].workersNeeded);

    int buildings_to_skip = calc_adjust_with_percentage(
        Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].buildings, percentage_not_filled);

    int workers_per_building;
    if (buildings_to_skip == Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].buildings) {
        workers_per_building = 1;
    } else {
        workers_per_building = Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].workersAllocated /
            (Data_CityInfo.laborCategory[LABOR_CATEGORY_WATER].buildings - buildings_to_skip);
    }
    int building_id = start_building_id;
    start_building_id = 0;
    for (int guard = 1; guard < MAX_BUILDINGS; guard++, building_id++) {
        if (building_id >= 2000) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (b->state != BUILDING_STATE_IN_USE || CATEGORY_FOR_BUILDING_TYPE[b->type] != LABOR_CATEGORY_WATER) {
            continue;
        }
        b->numWorkers = 0;
        if (b->percentageHousesCovered > 0) {
            if (percentage_not_filled > 0) {
                if (buildings_to_skip) {
                    --buildings_to_skip;
                } else if (start_building_id) {
                    b->numWorkers = workers_per_building;
                } else {
                    start_building_id = building_id;
                    b->numWorkers = workers_per_building;
                }
            } else {
                b->numWorkers = model_get_building(b->type)->laborers;
            }
        }
    }
    if (!start_building_id) {
        // no buildings assigned or full employment
        start_building_id = 1;
    }
}

static void allocate_workers_to_non_water_buildings()
{
    int category_workers_needed[MAX_CATS];
    int category_workers_allocated[MAX_CATS];
    for (int i = 0; i < MAX_CATS; i++) {
        category_workers_allocated[i] = 0;
        category_workers_needed[i] =
            Data_CityInfo.laborCategory[i].workersAllocated < Data_CityInfo.laborCategory[i].workersNeeded
            ? 1 : 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int cat = CATEGORY_FOR_BUILDING_TYPE[b->type];
        if (cat == LABOR_CATEGORY_WATER || cat < 0) {
            // water is handled by allocateWorkersToWater()
            continue;
        }
        b->numWorkers = 0;
        if (!should_have_workers(b, cat, 0)) {
            continue;
        }
        if (b->percentageHousesCovered > 0) {
            int required_workers = model_get_building(b->type)->laborers;
            if (category_workers_needed[cat]) {
                int num_workers = calc_adjust_with_percentage(
                    Data_CityInfo.laborCategory[cat].workersAllocated,
                    b->percentageHousesCovered) / 100;
                if (num_workers > required_workers) {
                    num_workers = required_workers;
                }
                b->numWorkers = num_workers;
                category_workers_allocated[cat] += num_workers;
            } else {
                b->numWorkers = required_workers;
            }
        }
    }
    for (int i = 0; i < MAX_CATS; i++) {
        if (category_workers_needed[i]) {
            // watch out: categoryWorkersNeeded is now reset to 'unallocated workers available'
            if (category_workers_allocated[i] >= Data_CityInfo.laborCategory[i].workersAllocated) {
                category_workers_needed[i] = 0;
                category_workers_allocated[i] = 0;
            } else {
                category_workers_needed[i] = Data_CityInfo.laborCategory[i].workersAllocated - category_workers_allocated[i];
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
        if (b->percentageHousesCovered > 0 && category_workers_needed[cat]) {
            int requiredWorkers = model_get_building(b->type)->laborers;
            if (b->numWorkers < requiredWorkers) {
                int needed = requiredWorkers - b->numWorkers;
                if (needed > category_workers_needed[cat]) {
                    b->numWorkers += category_workers_needed[cat];
                    category_workers_needed[cat] = 0;
                } else {
                    b->numWorkers += needed;
                    category_workers_needed[cat] -= needed;
                }
            }
        }
    }
}

static void allocate_workers_to_buildings()
{
    set_building_worker_weight();
    allocate_workers_to_water();
    allocate_workers_to_non_water_buildings();
}

void city_labor_allocate_workers()
{
    allocate_workers_to_categories();
    allocate_workers_to_buildings();
}


void city_labor_update()
{
    calculate_workers_needed_per_category();
    check_employment();
    allocate_workers_to_buildings();
}

void city_labor_set_priority(int category, int new_priority)
{
    int old_priority = Data_CityInfo.laborCategory[category].priority;
    if (old_priority == new_priority) {
        return;
    }
    int shift;
    int from_prio;
    int to_prio;
    if (!old_priority && new_priority) {
        // shift all bigger than 'newPriority' by one down (+1)
        shift = 1;
        from_prio = new_priority;
        to_prio = 9;
    } else if (old_priority && !new_priority) {
        // shift all bigger than 'oldPriority' by one up (-1)
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
    Data_CityInfo.laborCategory[category].priority = new_priority;
    for (int i = 0; i < 9; i++) {
        if (i == category) {
            continue;
        }
        int current_priority = Data_CityInfo.laborCategory[i].priority;
        if (from_prio <= current_priority && current_priority <= to_prio) {
            Data_CityInfo.laborCategory[i].priority += shift;
        }
    }
    city_labor_allocate_workers();
}

int city_labor_max_selectable_priority(int category)
{
    int max = 0;
    for (int i = 0; i < 9; i++) {
        if (Data_CityInfo.laborCategory[i].priority > 0) {
            ++max;
        }
    }
    if (max < 9 && !Data_CityInfo.laborCategory[category].priority) {
        // allow space for new priority
        ++max;
    }
    return max;
}
