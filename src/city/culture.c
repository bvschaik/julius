#include "culture.h"

#include "building/building.h"
#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/entertainment.h"
#include "city/festival.h"
#include "city/population.h"
#include "core/calc.h"

#include "Data/CityInfo.h"

static struct {
    int theater;
    int amphitheater;
    int colosseum;
    int hippodrome;
    int hospital;
    int school;
    int academy;
    int library;
    int religion[5];
    int oracle;
} coverage;

int city_culture_coverage_theater()
{
    return coverage.theater;
}

int city_culture_coverage_amphitheater()
{
    return coverage.amphitheater;
}

int city_culture_coverage_colosseum()
{
    return coverage.colosseum;
}

int city_culture_coverage_hippodrome()
{
    return coverage.hippodrome;
}

int city_culture_coverage_average_entertainment()
{
    return (coverage.hippodrome + coverage.colosseum + coverage.amphitheater + coverage.theater) / 4;
}

int city_culture_coverage_religion(god_type god)
{
    return coverage.religion[god];
}

int city_culture_coverage_school()
{
    return coverage.school;
}

int city_culture_coverage_library()
{
    return coverage.library;
}

int city_culture_coverage_academy()
{
    return coverage.academy;
}

int city_culture_coverage_hospital()
{
    return coverage.hospital;
}

static int top(int input)
{
    return input > 100 ? 100 : input;
}

void city_culture_update_coverage()
{
    int population = city_data.population.population;

    // entertainment
    coverage.theater = top(calc_percentage(500 * building_count_active(BUILDING_THEATER), population));
    coverage.amphitheater = top(calc_percentage(800 * building_count_active(BUILDING_AMPHITHEATER), population));
    coverage.colosseum = top(calc_percentage(1500 * building_count_active(BUILDING_COLOSSEUM), population));
    if (building_count_active(BUILDING_HIPPODROME) <= 0) {
        coverage.hippodrome = 0;
    } else {
        coverage.hippodrome = 100;
    }

    // religion
    int oracles = building_count_total(BUILDING_ORACLE);
    coverage.religion[GOD_CERES] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_CERES) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_CERES),
        population));
    coverage.religion[GOD_NEPTUNE] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_NEPTUNE) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_NEPTUNE),
        population));
    coverage.religion[GOD_MERCURY] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_MERCURY) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_MERCURY),
        population));
    coverage.religion[GOD_MARS] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_MARS) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_MARS),
        population));
    coverage.religion[GOD_VENUS] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_VENUS) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_VENUS),
        population));
    coverage.oracle = top(calc_percentage(500 * oracles, population));

    Data_CityInfo.cultureCoverageReligion =
        coverage.religion[GOD_CERES] +
        coverage.religion[GOD_NEPTUNE] +
        coverage.religion[GOD_MERCURY] +
        coverage.religion[GOD_MARS] +
        coverage.religion[GOD_VENUS];
    Data_CityInfo.cultureCoverageReligion /= 5;

    // education
    city_population_calculate_educational_age();

    coverage.school = top(calc_percentage(
        75 * building_count_active(BUILDING_SCHOOL), city_population_school_age()));
    coverage.library = top(calc_percentage(
        800 * building_count_active(BUILDING_LIBRARY), population));
    coverage.academy = top(calc_percentage(
        100 * building_count_active(BUILDING_ACADEMY), city_population_academy_age()));

    // health
    coverage.hospital = top(calc_percentage(
        1000 * building_count_active(BUILDING_HOSPITAL), population));
}

void city_culture_calculate()
{
    Data_CityInfo.citywideAverageEntertainment = 0;
    Data_CityInfo.citywideAverageReligion = 0;
    Data_CityInfo.citywideAverageEducation = 0;
    Data_CityInfo.citywideAverageHealth = 0;

    int num_houses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            num_houses++;
            Data_CityInfo.citywideAverageEntertainment += b->data.house.entertainment;
            Data_CityInfo.citywideAverageReligion += b->data.house.numGods;
            Data_CityInfo.citywideAverageEducation += b->data.house.education;
            Data_CityInfo.citywideAverageHealth += b->data.house.health;
        }
    }
    if (num_houses) {
        Data_CityInfo.citywideAverageEntertainment /= num_houses;
        Data_CityInfo.citywideAverageReligion /= num_houses;
        Data_CityInfo.citywideAverageEducation /= num_houses;
        Data_CityInfo.citywideAverageHealth /= num_houses;
    }

    city_entertainment_calculate_shows();
    city_festival_calculate_costs();
}

void city_culture_calculate_demands()
{
    // health
    Data_CityInfo.healthDemand = 0;
    int max = 0;
    if (city_data.houses.missing.bathhouse > max) {
        Data_CityInfo.healthDemand = 1;
        max = city_data.houses.missing.bathhouse;
    }
    if (city_data.houses.missing.barber > max) {
        Data_CityInfo.healthDemand = 2;
        max = city_data.houses.missing.barber;
    }
    if (city_data.houses.missing.clinic > max) {
        Data_CityInfo.healthDemand = 3;
        max = city_data.houses.missing.clinic;
    }
    if (city_data.houses.missing.hospital > max) {
        Data_CityInfo.healthDemand = 4;
    }
    // education
    Data_CityInfo.educationDemand = 0;
    if (city_data.houses.missing.more_education > city_data.houses.missing.education) {
        Data_CityInfo.educationDemand = 1; // schools(academies?)
    } else if (city_data.houses.missing.more_education == city_data.houses.missing.education) {
        Data_CityInfo.educationDemand = 2; // libraries
    } else if (city_data.houses.missing.more_education || city_data.houses.missing.education) {
        Data_CityInfo.educationDemand = 3; // more education
    }
    // entertainment
    Data_CityInfo.entertainmentDemand = 0;
    if (city_data.houses.missing.entertainment > city_data.houses.missing.more_entertainment) {
        Data_CityInfo.entertainmentDemand = 1;
    } else if (city_data.houses.missing.more_entertainment) {
        Data_CityInfo.entertainmentDemand = 2;
    }
    // religion
    Data_CityInfo.religionDemand = 0;
    max = 0;
    if (city_data.houses.missing.religion > max) {
        Data_CityInfo.religionDemand = 1;
        max = city_data.houses.missing.religion;
    }
    if (city_data.houses.missing.second_religion > max) {
        Data_CityInfo.religionDemand = 2;
        max = city_data.houses.missing.second_religion;
    }
    if (city_data.houses.missing.third_religion > max) {
        Data_CityInfo.religionDemand = 3;
    }
}

void city_culture_save_state(buffer *buf)
{
    // Yes, hospital is saved twice
    buffer_write_i32(buf, coverage.theater);
    buffer_write_i32(buf, coverage.amphitheater);
    buffer_write_i32(buf, coverage.colosseum);
    buffer_write_i32(buf, coverage.hospital);
    buffer_write_i32(buf, coverage.hippodrome);
    for (int i = GOD_CERES; i <= GOD_VENUS; i++) {
        buffer_write_i32(buf, coverage.religion[i]);
    }
    buffer_write_i32(buf, coverage.oracle);
    buffer_write_i32(buf, coverage.school);
    buffer_write_i32(buf, coverage.library);
    buffer_write_i32(buf, coverage.academy);
    buffer_write_i32(buf, coverage.hospital);
}

void city_culture_load_state(buffer *buf)
{
    // Yes, hospital is saved twice
    coverage.theater = buffer_read_i32(buf);
    coverage.amphitheater = buffer_read_i32(buf);
    coverage.colosseum = buffer_read_i32(buf);
    coverage.hospital = buffer_read_i32(buf);
    coverage.hippodrome = buffer_read_i32(buf);
    for (int i = GOD_CERES; i <= GOD_VENUS; i++) {
        coverage.religion[i] = buffer_read_i32(buf);
    }
    coverage.oracle = buffer_read_i32(buf);
    coverage.school = buffer_read_i32(buf);
    coverage.library = buffer_read_i32(buf);
    coverage.academy = buffer_read_i32(buf);
    coverage.hospital = buffer_read_i32(buf);
}

