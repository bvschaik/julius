#include "health.h"

#include "building/destruction.h"
#include "building/granary.h"
#include "building/warehouse.h"
#include "city/culture.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/tutorial.h"
#include "scenario/property.h"

#define NUM_PLAGUE_BUILDINGS sizeof(PLAGUE_BUILDINGS) / sizeof(building_type)

static const building_type PLAGUE_BUILDINGS[] = { BUILDING_DOCK, BUILDING_WAREHOUSE, BUILDING_GRANARY };


int city_health(void)
{
    return city_data.health.value;
}

void city_health_change(int amount)
{
    city_data.health.value = calc_bound(city_data.health.value + amount, 0, 100);
}

static void cause_plague_in_building(int building_id)
{
    building *b = building_get(building_id);
    if (!b->has_plague) {

        // remove all eatable resources from granary and warehouse
        if (b->type == BUILDING_GRANARY) {
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                building_granary_remove_resource(b, r, FULL_GRANARY);
            }
        }

        if (b->type == BUILDING_WAREHOUSE) {
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_IRON; r++) {
                building_warehouse_remove_resource(b, r, FULL_WAREHOUSE);
            }
        }

        // Set building to plague status and use fire process to manage plague on it
        b->has_plague = 1;
        b->sickness_duration = 0;

        if (b->type == BUILDING_WAREHOUSE || b->type == BUILDING_GRANARY || b->type == BUILDING_DOCK) {
            city_message_post(1, MESSAGE_SICKNESS, b->type, b->grid_offset);
        }
    }
}

void city_health_update_sickness_level_in_building(int building_id)
{
    building *b = building_get(building_id);

    if (!b->has_plague && b->state == BUILDING_STATE_IN_USE) {
        b->sickness_level += 1;

        if (b->sickness_level > MAX_SICKNESS_LEVEL) {
            b->sickness_level = MAX_SICKNESS_LEVEL;
        }
    }
}

void city_health_dispatch_sickness(figure *f)
{
    building *b = building_get(f->building_id);
    building *dest_b = building_get(f->destination_building_id);

    // dispatch sickness level sub value between granaries, warehouses and docks
    if ((dest_b->type == BUILDING_GRANARY || dest_b->type == BUILDING_WAREHOUSE || dest_b->type == BUILDING_DOCK) &&
        b->sickness_level && b->sickness_level > dest_b->sickness_level) {
        int value = b->sickness_level == 1 ? 1 : b->sickness_level / 2;
        if (dest_b->sickness_level < value) {
            dest_b->sickness_level = value;
        }
    } else if((b->type == BUILDING_GRANARY || b->type == BUILDING_WAREHOUSE || b->type == BUILDING_DOCK) &&
        dest_b->sickness_level && dest_b->sickness_level > b->sickness_level) {
        int value = dest_b->sickness_level == 1 ? 1 : dest_b->sickness_level / 2;
        if (b->sickness_level < value) {
            b->sickness_level = value;
        }
    }
}

static int cause_plague(void)
{
    int sick_people = 0;
    // kill people who have sickness level to max in houses
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type = 0; // building_destroy_by_plague changes the building type
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                if (b->sickness_level == MAX_SICKNESS_LEVEL) {
                    sick_people = 1;
                    building_destroy_by_plague(b);
                }
            }
        }
    }

    if (sick_people) {
        city_message_post(1, MESSAGE_HEALTH_PESTILENCE, 0, 0);
    }

    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->sickness_level == MAX_SICKNESS_LEVEL) {
                cause_plague_in_building(b->id);
            }
        }
    }

    return sick_people;
}

static void cause_disease(int total_people)
{
    if (cause_plague()) {
        return;
    }

    if (city_data.health.value >= 40) {
        return;
    }
    int chance_value = random_byte() & 0x3f;
    if (city_data.religion.venus_curse_active) {
        // force plague
        chance_value = 0;
        city_data.religion.venus_curse_active = 0;
    }
    if (chance_value > 40 - city_data.health.value) {
        return;
    }

    int sick_people = calc_adjust_with_percentage(total_people, 7 + (random_byte() & 3));
    if (sick_people <= 0) {
        return;
    }
    city_health_change(10);
    int people_to_kill = sick_people - city_data.health.num_hospital_workers;
    if (people_to_kill <= 0) {
        city_message_post(1, MESSAGE_HEALTH_ILLNESS, 0, 0);
        return;
    }
    if (city_data.health.num_hospital_workers > 0) {
        city_message_post(1, MESSAGE_HEALTH_DISEASE, 0, 0);
    } else {
        city_message_post(1, MESSAGE_HEALTH_PESTILENCE, 0, 0);
    }
    tutorial_on_disease();
    // kill people who don't have access to a doctor
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type = 0; // building_destroy_by_plague changes the building type
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                if (!b->data.house.clinic) {
                    people_to_kill -= b->house_population;
                    building_destroy_by_plague(b);
                    if (people_to_kill <= 0) {
                        return;
                    }
                }
            }
        }
    }
    // kill people in tents
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type = 0; // building_destroy_by_plague changes the building type
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                people_to_kill -= b->house_population;
                building_destroy_by_plague(b);
                if (people_to_kill <= 0) {
                    return;
                }
            }
        }
    }
    // kill anyone
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type = 0; // building_destroy_by_plague changes the building type
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                people_to_kill -= b->house_population;
                building_destroy_by_plague(b);
                if (people_to_kill <= 0) {
                    return;
                }
            }
        }
    }
}

static int get_sickness_reduction_ratio(void)
{
    int city_global_coverage_percent = city_health();
    int city_hospital_coverage_percent = city_culture_coverage_hospital();
    int base_hospital_coverage = 750;
    int population = city_population();

    if (population < 1000) { // before 1000 hab there is no malus
        city_hospital_coverage_percent = 0;
    } else {
        // there are hospitals, use real coverage to get malus
        if (city_hospital_coverage_percent) {
            city_hospital_coverage_percent = 100 - city_hospital_coverage_percent;
        } else { // else use base coverage
            city_hospital_coverage_percent = 100 - calc_percentage(base_hospital_coverage, population);
        }
    }

    int reduction_ratio = city_global_coverage_percent - city_hospital_coverage_percent;

    // can't have a raise of sickness_level in case of healing level is poor reduction is set to 0
    if (reduction_ratio < 0) {
        reduction_ratio = 0;                    
    }

    return reduction_ratio;
}

static void increase_sickness_level_in_building(building *b, int reduction_ratio)
{
    if (!b->has_plague) {
        if(b->sickness_level) {
            // increase by 5
            b->sickness_level += 5;

            if (b->sickness_level > MAX_SICKNESS_LEVEL) {
                b->sickness_level = MAX_SICKNESS_LEVEL;
            }
            // then apply reduction
            b->sickness_level = b->sickness_level - (b->sickness_level * reduction_ratio / 100);
        }
    }
}

static void increase_sickness_level_in_buildings(void)
{
    int reduction_ratio = get_sickness_reduction_ratio();

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            increase_sickness_level_in_building(b, reduction_ratio);
        }
    }

    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            increase_sickness_level_in_building(b, reduction_ratio);
        }
    }
}

void city_health_update(void)
{
    if (city_data.population.population < 200 || scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
        city_data.health.value = 50;
        city_data.health.target_value = 50;
        return;
    }
    int total_population = 0;
    int healthy_population = 0;
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size || !b->house_population) {
                continue;
            }
            total_population += b->house_population;
            if (b->subtype.house_level <= HOUSE_LARGE_TENT) {
                if (b->data.house.clinic) {
                    healthy_population += b->house_population;
                } else {
                    healthy_population += b->house_population / 4;
                }
            } else if (b->data.house.clinic) {
                if (b->house_days_without_food == 0) {
                    healthy_population += b->house_population;
                } else {
                    healthy_population += b->house_population / 4;
                }
            } else if (b->house_days_without_food == 0) {
                healthy_population += b->house_population / 4;
            }
        }
    }
    city_data.health.target_value = calc_percentage(healthy_population, total_population);
    if (city_data.health.value < city_data.health.target_value) {
        city_data.health.value += 2;
        if (city_data.health.value > city_data.health.target_value) {
            city_data.health.value = city_data.health.target_value;
        }
    } else if (city_data.health.value > city_data.health.target_value) {
        city_data.health.value -= 2;
        if (city_data.health.value < city_data.health.target_value) {
            city_data.health.value = city_data.health.target_value;
        }
    }
    city_data.health.value = calc_bound(city_data.health.value, 0, 100);

    increase_sickness_level_in_buildings();
    cause_disease(total_population);
}

void city_health_reset_hospital_workers(void)
{
    city_data.health.num_hospital_workers = 0;
}

void city_health_add_hospital_workers(int amount)
{
    city_data.health.num_hospital_workers += amount;
}

int city_health_get_global_sickness_level(void)
{
    int building_number = 0;
    int building_sickness_level = 0;
    int max_sickness_level = 0;

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type;
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                building_number++;
                building_sickness_level += b->sickness_level;

                if (b->sickness_level > max_sickness_level) {
                    max_sickness_level = b->sickness_level;
                }
            }
        }
    }

    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            building_number++;
            building_sickness_level += b->sickness_level;
            if (b->sickness_level > max_sickness_level) {
                max_sickness_level = b->sickness_level;
            }
        }
    }

    int plague_incoming;

    if (max_sickness_level >= HIGH_SICKNESS_LEVEL) {
        plague_incoming = 2;
    } else if (max_sickness_level >= MEDIUM_SICKNESS_LEVEL) {
        plague_incoming = 1;
    } else {
        plague_incoming = 0;
    }

    if (building_number == 0) {
        return SICKNESS_LEVEL_LOW;
    }

    int global_rating = building_sickness_level / building_number;
    int global_sickness_level;

    if (global_rating < LOW_SICKNESS_LEVEL && !plague_incoming) {
        global_sickness_level = SICKNESS_LEVEL_LOW;
    } else if (global_rating < MEDIUM_SICKNESS_LEVEL && !plague_incoming) {
        global_sickness_level = SICKNESS_LEVEL_MEDIUM;
    } else if (global_rating < HIGH_SICKNESS_LEVEL && plague_incoming == 1) {
        global_sickness_level = SICKNESS_LEVEL_HIGH;
    } else {
        global_sickness_level = SICKNESS_LEVEL_PLAGUE;
    }

    return global_sickness_level;
}
