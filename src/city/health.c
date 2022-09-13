#include "health.h"

#include "building/count.h"
#include "building/destruction.h"
#include "building/granary.h"
#include "building/house.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/warehouse.h"
#include "city/culture.h"
#include "city/data_private.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/tutorial.h"
#include "scenario/property.h"

#define SICKNESS_SPREAD_DIVISION_FACTOR 4

#define NUM_PLAGUE_BUILDINGS (sizeof(PLAGUE_BUILDINGS) / sizeof(building_type))
static const building_type PLAGUE_BUILDINGS[] = { BUILDING_DOCK, BUILDING_WAREHOUSE, BUILDING_GRANARY };

int city_health(void)
{
    return city_data.health.value;
}

void city_health_change(int amount)
{
    city_data.health.value = calc_bound(city_data.health.value + amount, 0, 100);
}

static int is_plague_building(building_type type)
{
    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        if (type == PLAGUE_BUILDINGS[i]) {
            return 1;
        }
    }
    return 0;
}

static void cause_disease_in_building(int building_id)
{
    building *b = building_get(building_id);
    if (!b->has_plague) {

        // Remove half the granary's food
        if (b->type == BUILDING_GRANARY) {
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                building_granary_remove_resource(b, r, building_granary_resource_amount(r, b) / 2);
            }
        } else if (b->type == BUILDING_WAREHOUSE) {
            // Remove all food from warehouse
            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                building_warehouse_remove_resource(b, r, FULL_WAREHOUSE);
            }
            // Remove half of oil and wine from warehouse
            for (int r = RESOURCE_WINE; r <= RESOURCE_OIL; r++) {
                building_warehouse_remove_resource(b, r, building_warehouse_get_amount(b, r) / 2);
            }
        }

        // Set building to plague status and use fire process to manage plague on it
        b->has_plague = 1;
        b->sickness_duration = 0;

        if (is_plague_building(b->type)) {
            city_message_post(1, MESSAGE_SICKNESS, b->type, b->grid_offset);
        }
    }
}

void city_health_update_sickness_level_in_building(int building_id)
{
    building *b = building_get(building_id);

    if (!b->has_plague && b->state == BUILDING_STATE_IN_USE) {
        b->sickness_level += 1;

        if (b->sickness_level > 2 * MAX_SICKNESS_LEVEL) {
            b->sickness_level = 2 * MAX_SICKNESS_LEVEL;
        }
    }
}

void city_health_dispatch_sickness(figure *f)
{
    building *b = building_get(f->building_id);
    building *dest_b = building_get(f->destination_building_id);

    // Dispatch sickness level sub value between granaries, warehouses and docks
    if (is_plague_building(dest_b->type) && b->sickness_level && b->sickness_level > dest_b->sickness_level) {
        int value = b->sickness_level <= SICKNESS_SPREAD_DIVISION_FACTOR ? 1 :
            b->sickness_level / SICKNESS_SPREAD_DIVISION_FACTOR;
        dest_b->sickness_level += value;
        if (dest_b->sickness_level > b->sickness_level) {
            dest_b->sickness_level = b->sickness_level;
        }
    } else if (is_plague_building(b->type) && dest_b->sickness_level && dest_b->sickness_level > b->sickness_level) {
        int value = b->sickness_level <= SICKNESS_SPREAD_DIVISION_FACTOR ? 1 :
            b->sickness_level / SICKNESS_SPREAD_DIVISION_FACTOR;
        b->sickness_level += value;
        if (b->sickness_level > dest_b->sickness_level) {
            b->sickness_level = dest_b->sickness_level;
        }
    }
}

static int cause_disease(void)
{
    int sick_people = 0;
    building_type sick_building_type = 0;
    int grid_offset = 0;
    // Kill people who have sickness level to max in houses
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        building *next_of_type = 0; // building_destroy_by_plague changes the building type
        for (building *b = building_first_of_type(type); b; b = next_of_type) {
            next_of_type = b->next_of_type;
            if (b->state == BUILDING_STATE_IN_USE && b->house_size && b->house_population) {
                if (b->sickness_level >= MAX_SICKNESS_LEVEL) {
                    sick_people = 1;
                    sick_building_type = b->type;
                    grid_offset = b->grid_offset;
                    if (city_health() < 40) {
                        building_destroy_by_plague(b);
                    } else {
                        int killed_people = b->house_population -
                            calc_adjust_with_percentage(b->house_population, city_health());
                        if (killed_people == 0) {
                            killed_people = 1;
                        }
                        if (killed_people < b->house_population) {
                            b->house_population -= killed_people;
                        } else {
                            building_house_change_to_vacant_lot(b);
                        }
                        city_population_remove_home_removed(killed_people);

                        // Cause plague in the house
                        b->immigrant_figure_id = 0;
                        b->has_plague = 1;
                        b->sickness_duration = 0;
                    }
                }
            }
        }
    }

    if (sick_people) {
        city_message_post_with_popup_delay(MESSAGE_CAT_ILLNESS, MESSAGE_SICKNESS, sick_building_type, grid_offset);
    }

    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->sickness_level >= MAX_SICKNESS_LEVEL) {
                cause_disease_in_building(b->id);
            }
        }
    }

    return sick_people;
}

static void cause_plague(int total_people)
{
    if (cause_disease()) {
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
    // kill anyone, starting with tents and working up the housing levels
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

static void adjust_sickness_level_in_house(building *b, int health, int population_health_offset, int hospital_bonus)
{
    if (!b->has_plague && b->sickness_level) {
        int delta;
        // Case-specific health reduction when health is perfect and pop is high enough
        if (population_health_offset == 10 && health == 100) {
            delta = -1;
        } else {
            delta = 10 - (health / 10) * 2;
            delta += population_health_offset;
        }

        // Neptune GT reduces the delta by 5
        if (building_monument_working(BUILDING_GRAND_TEMPLE_NEPTUNE)) {
            delta -= 5;
        }

        // If delta is positive, it is reduced depending on house health, global health and hospital access
        if (delta > 0) {
            int delta_decrease_percentage = city_health();
            delta_decrease_percentage -= calc_adjust_with_percentage(city_health(), delta * 5);
            delta_decrease_percentage += calc_adjust_with_percentage(city_health(), hospital_bonus);
            delta -= calc_adjust_with_percentage(delta, delta_decrease_percentage);
        }

        b->sickness_level = calc_bound(b->sickness_level + delta, 0, MAX_SICKNESS_LEVEL);
    }
}

static void adjust_sickness_level_in_plague_buildings(int hospital_coverage_bonus)
{
    for (int i = 0; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->has_plague || !b->sickness_level) {
                continue;
            }
            int decrease_percentage = city_health();
            decrease_percentage += calc_adjust_with_percentage(decrease_percentage, hospital_coverage_bonus);
            if (decrease_percentage > 100) {
                decrease_percentage = 100;
            }
            b->sickness_level -= calc_adjust_with_percentage(b->sickness_level, decrease_percentage);
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
    int population_health_offset = calc_bound((city_population() - 1) / 1000, 0, 10);
    int hospital_coverage_bonus = city_culture_coverage_hospital() / 20 * 5;
    city_data.health.population_access.clinic = 0;
    city_data.health.population_access.barber = 0;
    city_data.health.population_access.baths = 0;

    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->state != BUILDING_STATE_IN_USE || !b->house_size) {
                continue;
            }
            if (!b->house_population) {
                b->sickness_level = 0;
                continue;
            }
            int house_health = calc_bound(b->subtype.house_level, 0, 10);
            if (b->data.house.clinic && b->data.house.hospital) {
                house_health += 50;
                city_data.health.population_access.clinic += b->house_population;
            } else if (b->data.house.hospital) {
                house_health += 40;
            } else if (b->data.house.clinic) {
                house_health += 30;
                city_data.health.population_access.clinic += b->house_population;
            }
            if (b->data.house.bathhouse) {
                house_health += 20;
                city_data.health.population_access.baths += b->house_population;
            }
            if (b->data.house.barber) {
                house_health += 10;
                city_data.health.population_access.barber += b->house_population;
            }
            house_health += b->data.house.num_foods * 15;

            int mausuleum_health = building_count_active(BUILDING_SMALL_MAUSOLEUM);
            mausuleum_health += building_count_active(BUILDING_LARGE_MAUSOLEUM) * 2;

            house_health += calc_bound(mausuleum_health, 0, 10);

            int health_cap = (model_get_house(b->subtype.house_level)->food_types && !b->data.house.num_foods) ?
                40 : 100;
            house_health = calc_bound(house_health, 0, health_cap);
            total_population += b->house_population;
            healthy_population += calc_adjust_with_percentage(b->house_population, house_health);
            adjust_sickness_level_in_house(b, house_health, population_health_offset, hospital_coverage_bonus);
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

    adjust_sickness_level_in_plague_buildings(hospital_coverage_bonus);

    cause_plague(total_population);
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
                building_sickness_level += calc_bound(b->sickness_level, 0, MAX_SICKNESS_LEVEL);

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
            building_sickness_level += calc_bound(b->sickness_level, 0, MAX_SICKNESS_LEVEL);
            if (b->sickness_level > max_sickness_level) {
                max_sickness_level = b->sickness_level;
            }
        }
    }

    if (max_sickness_level < MAX_SICKNESS_LEVEL) {
        for (building *b = building_first_of_type(BUILDING_BURNING_RUIN); b; b = b->next_of_type) {
            if (b->state == BUILDING_STATE_IN_USE && b->has_plague) {
                max_sickness_level = MAX_SICKNESS_LEVEL;
                break;
            }
        }
    }

    if (building_number == 0) {
        return SICKNESS_LEVEL_LOW;
    }

    int global_sickness_level = SICKNESS_LEVEL_LOW;

    if (max_sickness_level == MAX_SICKNESS_LEVEL) { // one or many houses is plagued
        global_sickness_level = SICKNESS_LEVEL_PLAGUE;
    } else if (max_sickness_level >= HIGH_SICKNESS_LEVEL) { // one or many houses have sickness_level >= 90
        global_sickness_level = SICKNESS_LEVEL_HIGH;
    } else if (max_sickness_level >= MEDIUM_SICKNESS_LEVEL) { // one or many houses have sickness_level >= 60
        global_sickness_level = SICKNESS_LEVEL_MEDIUM;
    }

    return global_sickness_level;
}

int city_health_get_population_with_clinic_access(void)
{
    return city_data.health.population_access.clinic;
}

int city_health_get_population_with_barber_access(void)
{
    return city_data.health.population_access.barber;
}

int city_health_get_population_with_baths_access(void)
{
    return city_data.health.population_access.baths;
}
