#include "population.h"

#include "building/house_population.h"
#include "city/data_private.h"
#include "core/calc.h"
#include "core/random.h"

#include "Data/CityInfo.h"

static const int BIRTHS_PER_AGE_DECENNIUM[10] = {
    0, 3, 16, 9, 2, 0, 0, 0, 0, 0
};

static const int DEATHS_PER_HEALTH_PER_AGE_DECENNIUM[11][10] = {
    {20, 10, 5, 10, 20, 30, 50, 85, 100, 100},
    {15, 8, 4, 8, 16, 25, 45, 70, 90, 100},
    {10, 6, 2, 6, 12, 20, 30, 55, 80, 90},
    {5, 4, 0, 4, 8, 15, 25, 40, 65, 80},
    {3, 2, 0, 2, 6, 12, 20, 30, 50, 70},
    {2, 0, 0, 0, 4, 8, 15, 25, 40, 60},
    {1, 0, 0, 0, 2, 6, 12, 20, 30, 50},
    {0, 0, 0, 0, 0, 4, 8, 15, 20, 40},
    {0, 0, 0, 0, 0, 2, 6, 10, 15, 30},
    {0, 0, 0, 0, 0, 0, 4, 5, 10, 20},
    {0, 0, 0, 0, 0, 0, 0, 2, 5, 10}
};

int city_population()
{
    return city_data.population.population;
}

int city_population_school_age()
{
    return city_data.population.school_age;
}

int city_population_academy_age()
{
    return city_data.population.academy_age;
}

static void recalculate_population()
{
    city_data.population.population = 0;
    for (int i = 0; i < 100; i++) {
        city_data.population.population += Data_CityInfo.populationPerAge[i];
    }
    if (city_data.population.population > Data_CityInfo.populationHighestEver) {
        Data_CityInfo.populationHighestEver = city_data.population.population;
    }
}

static void add_to_census(int num_people)
{
    int odd = 0;
    int index = 0;
    for (int i = 0; i < num_people; i++, odd = 1 - odd) {
        int age = random_from_pool(index++) & 0x3f; // 63
        if (age > 50) {
            age -= 30;
        } else if (age < 10 && odd) {
            age += 20;
        }
        Data_CityInfo.populationPerAge[age]++;
    }
}

static void remove_from_census(int num_people)
{
    int index = 0;
    int empty_buckets = 0;
    // remove people randomly up to age 63
    while (num_people > 0 && empty_buckets < 100) {
        int age = random_from_pool(index++) & 0x3f;
        if (Data_CityInfo.populationPerAge[age] <= 0) {
            empty_buckets++;
        } else {
            Data_CityInfo.populationPerAge[age]--;
            num_people--;
            empty_buckets = 0;
        }
    }
    // if random didn't work: remove from age 10 and up
    empty_buckets = 0;
    int age = 10;
    while (num_people > 0 && empty_buckets < 100) {
        if (Data_CityInfo.populationPerAge[age] <= 0) {
            empty_buckets++;
        } else {
            Data_CityInfo.populationPerAge[age]--;
            num_people--;
            empty_buckets = 0;
        }
        age++;
        if (age >= 100) {
            age = 0;
        }
    }
}

static void remove_from_census_in_age_decennium(int decennium, int num_people)
{
    int empty_buckets = 0;
    int age = 0;
    while (num_people > 0 && empty_buckets < 10) {
        if (Data_CityInfo.populationPerAge[10 * decennium + age] <= 0) {
            empty_buckets++;
        } else {
            Data_CityInfo.populationPerAge[10 * decennium + age]--;
            num_people--;
            empty_buckets = 0;
        }
        age++;
        if (age >= 10) {
            age = 0;
        }
    }
}

static int get_people_in_age_decennium(int decennium)
{
    int pop = 0;
    for (int i = 0; i < 10; i++) {
        pop += Data_CityInfo.populationPerAge[10 * decennium + i];
    }
    return pop;
}

void city_population_add(int num_people)
{
    Data_CityInfo.populationLastChange = num_people;
    add_to_census(num_people);
    recalculate_population();
}

void city_population_remove(int num_people)
{
    Data_CityInfo.populationLastChange = -num_people;
    remove_from_census(num_people);
    recalculate_population();
}

void city_population_add_homeless(int num_people)
{
    Data_CityInfo.populationLostHomeless -= num_people;
    add_to_census(num_people);
    recalculate_population();
}

void city_population_remove_homeless(int num_people)
{
    Data_CityInfo.populationLostHomeless += num_people;
    remove_from_census(num_people);
    recalculate_population();
}

void city_population_remove_home_removed(int num_people)
{
    Data_CityInfo.populationLostInRemoval += num_people;
    remove_from_census(num_people);
    recalculate_population();
}

void city_population_remove_for_troop_request(int num_people)
{
    int removed = house_population_remove_from_city(num_people);
    remove_from_census(removed);
    Data_CityInfo.populationLostTroopRequest += num_people;
    recalculate_population();
}

int city_population_people_of_working_age()
{
    return
        get_people_in_age_decennium(2) +
        get_people_in_age_decennium(3) +
        get_people_in_age_decennium(4);
}

static int get_people_aged_between(int min, int max)
{
    int pop = 0;
    for (int i = min; i < max; i++) {
        pop += Data_CityInfo.populationPerAge[i];
    }
    return pop;
}

void city_population_calculate_educational_age()
{
    city_data.population.school_age = get_people_aged_between(0, 14);
    city_data.population.academy_age = get_people_aged_between(14, 21);
}

void city_population_record_monthly()
{
    Data_CityInfo.monthlyPopulation[Data_CityInfo.monthlyPopulationNextIndex++] = city_data.population.population;
    if (Data_CityInfo.monthlyPopulationNextIndex >= 2400) {
        Data_CityInfo.monthlyPopulationNextIndex = 0;
    }
    ++Data_CityInfo.monthsSinceStart;
}

static void yearly_advance_ages_and_calculate_deaths()
{
    int aged100 = Data_CityInfo.populationPerAge[99];
    for (int age = 99; age > 0; age--) {
        Data_CityInfo.populationPerAge[age] = Data_CityInfo.populationPerAge[age-1];
    }
    Data_CityInfo.populationPerAge[0] = 0;
    Data_CityInfo.populationYearlyDeaths = 0;
    for (int decennium = 9; decennium >= 0; decennium--) {
        int people = get_people_in_age_decennium(decennium);
        int death_percentage = DEATHS_PER_HEALTH_PER_AGE_DECENNIUM[Data_CityInfo.healthRate / 10][decennium];
        int deaths = calc_adjust_with_percentage(people, death_percentage);
        int removed = house_population_remove_from_city(deaths + aged100);
        remove_from_census_in_age_decennium(decennium, removed);
        // ^ BUGFIX should be deaths only, now aged100 are removed from census while they weren't *in* the census anymore
        Data_CityInfo.populationYearlyDeaths += removed;
        aged100 = 0;
    }
}

static void yearly_calculate_births()
{
    Data_CityInfo.populationYearlyBirths = 0;
    for (int decennium = 9; decennium >= 0; decennium--) {
        int people = get_people_in_age_decennium(decennium);
        int births = calc_adjust_with_percentage(people, BIRTHS_PER_AGE_DECENNIUM[decennium]);
        int added = house_population_add_to_city(births);
        Data_CityInfo.populationPerAge[0] += added;
        Data_CityInfo.populationYearlyBirths += added;
    }
}

static void yearly_recalculate_population()
{
    Data_CityInfo.populationYearlyUpdatedNeeded = 0;
    city_data.population.population_last_year = city_data.population.population;
    recalculate_population();

    Data_CityInfo.populationLostInRemoval = 0;
    Data_CityInfo.populationTotalAllYears += city_data.population.population;
    Data_CityInfo.populationTotalYears++;
    Data_CityInfo.populationAveragePerYear = Data_CityInfo.populationTotalAllYears / Data_CityInfo.populationTotalYears;
}

void city_population_request_yearly_update()
{
    Data_CityInfo.populationYearlyUpdatedNeeded = 1;
    house_population_calculate_people_per_type();
}

void city_population_yearly_update()
{
    if (Data_CityInfo.populationYearlyUpdatedNeeded) {
        yearly_advance_ages_and_calculate_deaths();
        yearly_calculate_births();
        yearly_recalculate_population();
    }
}

void city_population_check_consistency()
{
    int people_in_houses = house_population_calculate_people_per_type();
    if (people_in_houses < city_data.population.population) {
        remove_from_census(city_data.population.population - people_in_houses);
    }
}
