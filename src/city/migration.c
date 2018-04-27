#include "migration.h"

#include "building/house_population.h"
#include "city/data_private.h"
#include "city/figures.h"
#include "city/message.h"
#include "core/calc.h"
#include "game/tutorial.h"

#include "Data/CityInfo.h"

static void update_status()
{
    if (city_data.sentiment.value > 70) {
        city_data.migration.percentage = 100;
    } else if (city_data.sentiment.value > 60) {
        city_data.migration.percentage = 75;
    } else if (city_data.sentiment.value >= 50) {
        city_data.migration.percentage = 50;
    } else if (city_data.sentiment.value > 40) {
        city_data.migration.percentage = 0;
    } else if (city_data.sentiment.value > 30) {
        city_data.migration.percentage = -10;
    } else if (city_data.sentiment.value > 20) {
        city_data.migration.percentage = -25;
    } else {
        city_data.migration.percentage = -50;
    }

    city_data.migration.immigration_amount_per_batch = 0;
    city_data.migration.emigration_amount_per_batch = 0;

    int population_cap = tutorial_get_population_cap(200000);
    if (city_data.population.population >= population_cap) {
        city_data.migration.percentage = 0;
        return;
    }
    // war scares immigrants away
    if (city_figures_total_invading_enemies() > 3 &&
        city_data.migration.percentage > 0) {
        city_data.migration.percentage = 0;
        return;
    }
    if (city_data.migration.percentage > 0) {
        // immigration
        if (city_data.migration.emigration_duration) {
            city_data.migration.emigration_duration--;
        } else {
            city_data.migration.immigration_amount_per_batch =
                calc_adjust_with_percentage(12, city_data.migration.percentage);
            city_data.migration.immigration_duration = 2;
        }
    } else if (city_data.migration.percentage < 0) {
        // emigration
        if (city_data.migration.immigration_duration) {
            city_data.migration.immigration_duration--;
        } else if (city_data.population.population > 100) {
            city_data.migration.emigration_amount_per_batch =
                calc_adjust_with_percentage(12, -city_data.migration.percentage);
            city_data.migration.emigration_duration = 2;
        }
    }
}

static void create_immigrants(int num_people)
{
    int immigrated = house_population_create_immigrants(num_people);
    city_data.migration.immigrated_today += immigrated;
    city_data.migration.newcomers += city_data.migration.immigrated_today;
    if (immigrated == 0) {
        city_data.migration.refused_immigrants_today += num_people;
    }
}

static void create_emigrants(int num_people)
{
    city_data.migration.emigrated_today += house_population_create_emigrants(num_people);
}

static void create_migrants()
{
    city_data.migration.immigrated_today = 0;
    city_data.migration.emigrated_today = 0;
    city_data.migration.refused_immigrants_today = 0;

    if (city_data.migration.immigration_amount_per_batch > 0) {
        if (city_data.migration.immigration_amount_per_batch >= 4) {
            create_immigrants(city_data.migration.immigration_amount_per_batch);
        } else if (city_data.migration.immigration_amount_per_batch + city_data.migration.immigration_queue_size >= 4) {
            create_immigrants(city_data.migration.immigration_amount_per_batch + city_data.migration.immigration_queue_size);
            city_data.migration.immigration_queue_size = 0;
        } else {
            // queue them for next round
            city_data.migration.immigration_queue_size += city_data.migration.immigration_amount_per_batch;
        }
    }
    if (city_data.migration.emigration_amount_per_batch > 0) {
        if (city_data.migration.emigration_amount_per_batch >= 4) {
            create_emigrants(city_data.migration.emigration_amount_per_batch);
        } else if (city_data.migration.emigration_amount_per_batch + city_data.migration.emigration_queue_size >= 4) {
            create_emigrants(city_data.migration.emigration_amount_per_batch + city_data.migration.emigration_queue_size);
            city_data.migration.emigration_queue_size = 0;
            if (!city_data.migration.emigration_message_shown) {
                city_data.migration.emigration_message_shown = 1;
                city_message_post(1, MESSAGE_EMIGRATION, 0, 0);
            }
        } else {
            // queue them for next round
            city_data.migration.emigration_queue_size += city_data.migration.emigration_amount_per_batch;
        }
    }
    city_data.migration.immigration_amount_per_batch = 0;
    city_data.migration.emigration_amount_per_batch = 0;
}

void city_migration_update()
{
    update_status();
    create_migrants();
}

void city_migration_determine_no_immigration_cause()
{
    switch (city_data.sentiment.low_mood_cause) {
        case LOW_MOOD_CAUSE_NO_FOOD:
            city_data.migration.no_immigration_cause = 2;
            break;
        case LOW_MOOD_CAUSE_NO_JOBS:
            city_data.migration.no_immigration_cause = 1;
            break;
        case LOW_MOOD_CAUSE_HIGH_TAXES:
            city_data.migration.no_immigration_cause = 3;
            break;
        case LOW_MOOD_CAUSE_LOW_WAGES:
            city_data.migration.no_immigration_cause = 0;
            break;
        case LOW_MOOD_CAUSE_MANY_TENTS:
            city_data.migration.no_immigration_cause = 4;
            break;
        default:
            city_data.migration.no_immigration_cause = 5;
            break;
    }
}

int city_migration_no_immigation_cause()
{
    return city_data.migration.no_immigration_cause;
}

int city_migration_no_room_for_immigrants()
{
    return city_data.migration.refused_immigrants_today || Data_CityInfo.populationRoomInHouses <= 0;
}

int city_migration_percentage()
{
    return city_data.migration.percentage;
}

int city_migration_newcomers()
{
    return city_data.migration.newcomers;
}

void city_migration_reset_newcomers()
{
    city_data.migration.newcomers = 0;
}
