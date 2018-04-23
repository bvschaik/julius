#include "migration.h"

#include "building/house_population.h"
#include "city/data_private.h"
#include "city/message.h"
#include "core/calc.h"
#include "game/tutorial.h"

#include "Data/CityInfo.h"

static void update_status()
{
    if (city_data.sentiment.value > 70) {
        Data_CityInfo.populationMigrationPercentage = 100;
    } else if (city_data.sentiment.value > 60) {
        Data_CityInfo.populationMigrationPercentage = 75;
    } else if (city_data.sentiment.value >= 50) {
        Data_CityInfo.populationMigrationPercentage = 50;
    } else if (city_data.sentiment.value > 40) {
        Data_CityInfo.populationMigrationPercentage = 0;
    } else if (city_data.sentiment.value > 30) {
        Data_CityInfo.populationMigrationPercentage = -10;
    } else if (city_data.sentiment.value > 20) {
        Data_CityInfo.populationMigrationPercentage = -25;
    } else {
        Data_CityInfo.populationMigrationPercentage = -50;
    }

    city_data.migration.immigration_amount_per_batch = 0;
    city_data.migration.emigration_amount_per_batch = 0;

    int population_cap = tutorial_get_population_cap(200000);
    if (city_data.population.population >= population_cap) {
        Data_CityInfo.populationMigrationPercentage = 0;
        return;
    }
    // war scares immigrants away
    if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 3 &&
        Data_CityInfo.populationMigrationPercentage > 0) {
        Data_CityInfo.populationMigrationPercentage = 0;
        return;
    }
    if (Data_CityInfo.populationMigrationPercentage > 0) {
        // immigration
        if (city_data.population.emigration_duration) {
            city_data.population.emigration_duration--;
        } else {
            city_data.migration.immigration_amount_per_batch =
                calc_adjust_with_percentage(12, Data_CityInfo.populationMigrationPercentage);
            city_data.population.immigration_duration = 2;
        }
    } else if (Data_CityInfo.populationMigrationPercentage < 0) {
        // emigration
        if (city_data.population.immigration_duration) {
            city_data.population.immigration_duration--;
        } else if (city_data.population.population > 100) {
            city_data.migration.emigration_amount_per_batch =
                calc_adjust_with_percentage(12, -Data_CityInfo.populationMigrationPercentage);
            city_data.population.emigration_duration = 2;
        }
    }
}

static void create_immigrants(int num_people)
{
    int immigrated = house_population_create_immigrants(num_people);
    city_data.migration.immigrated_today += immigrated;
    Data_CityInfo.populationNewcomersThisMonth += city_data.migration.immigrated_today;
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
            if (!Data_CityInfo.messageShownEmigration) {
                Data_CityInfo.messageShownEmigration = 1;
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
