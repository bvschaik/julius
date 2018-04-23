#include "health.h"

#include "building/building.h"
#include "building/destruction.h"
#include "city/data_private.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/tutorial.h"
#include "scenario/property.h"

int city_health()
{
    return city_data.health.value;
}

void city_health_change(int amount)
{
    city_data.health.value = calc_bound(city_data.health.value + amount, 0, 100);
}

static void cause_disease(int total_people)
{
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
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize && b->housePopulation) {
            if (!b->data.house.clinic) {
                people_to_kill -= b->housePopulation;
                building_destroy_by_plague(b);
                if (people_to_kill <= 0) {
                    return;
                }
            }
        }
    }
    // kill people in tents
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize && b->housePopulation) {
            if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
                people_to_kill -= b->housePopulation;
                building_destroy_by_plague(b);
                if (people_to_kill <= 0) {
                    return;
                }
            }
        }
    }
    // kill anyone
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize && b->housePopulation) {
            people_to_kill -= b->housePopulation;
            building_destroy_by_plague(b);
            if (people_to_kill <= 0) {
                return;
            }
        }
    }
}

void city_health_update()
{
    if (city_data.population.population < 200 || scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
        city_data.health.value = 50;
        city_data.health.target_value = 50;
        return;
    }
    int total_population = 0;
    int healthy_population = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || !b->houseSize || !b->housePopulation) {
            continue;
        }
        total_population += b->housePopulation;
        if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
            if (b->data.house.clinic) {
                healthy_population += b->housePopulation;
            } else {
                healthy_population += b->housePopulation / 4;
            }
        } else if (b->data.house.clinic) {
            if (b->houseDaysWithoutFood == 0) {
                healthy_population += b->housePopulation;
            } else {
                healthy_population += b->housePopulation / 4;
            }
        } else if (b->houseDaysWithoutFood == 0) {
            healthy_population += b->housePopulation / 4;
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

    cause_disease(total_population);
}

void city_health_reset_hospital_workers()
{
    city_data.health.num_hospital_workers = 0;
}

void city_health_add_hospital_workers(int amount)
{
    city_data.health.num_hospital_workers += amount;
}
