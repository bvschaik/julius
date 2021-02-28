#include "culture.h"

#include "building/building.h"
#include "building/count.h"
#include "building/monument.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/entertainment.h"
#include "city/festival.h"
#include "city/population.h"
#include "core/calc.h"



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
    int tavern;
    int arena;
} coverage;

int city_culture_coverage_tavern(void)
{
    return coverage.tavern;
}

int city_culture_coverage_theater(void)
{
    return coverage.theater;
}

int city_culture_coverage_amphitheater(void)
{
    return coverage.amphitheater;
}

int city_culture_coverage_arena(void)
{
    return coverage.arena;
}

int city_culture_coverage_colosseum(void)
{
    return coverage.colosseum;
}

int city_culture_coverage_hippodrome(void)
{
    return coverage.hippodrome;
}

int city_culture_coverage_average_entertainment(void)
{
    return (coverage.hippodrome + coverage.colosseum + coverage.amphitheater + coverage.theater + coverage.tavern) / 5;
}

int city_culture_coverage_religion(god_type god)
{
    return coverage.religion[god];
}

int city_culture_coverage_school(void)
{
    return coverage.school;
}

int city_culture_coverage_library(void)
{
    return coverage.library;
}

int city_culture_coverage_academy(void)
{
    return coverage.academy;
}

int city_culture_coverage_hospital(void)
{
    return coverage.hospital;
}

int city_culture_average_education(void)
{
    return city_data.culture.average_education;
}

int city_culture_average_entertainment(void)
{
    return city_data.culture.average_entertainment;
}

int city_culture_average_health(void)
{
    return city_data.culture.average_health;
}

static int top(int input)
{
    return input > 100 ? 100 : input;
}

void city_culture_update_coverage(void)
{
    int population = city_data.population.population;

    // entertainment
    coverage.tavern = top(calc_percentage(city_culture_get_theatre_person_coverage(), population));
    coverage.theater = top(calc_percentage(THEATER_COVERAGE * building_count_active(BUILDING_THEATER), population));
    coverage.amphitheater = top(calc_percentage(AMPHITHEATER_COVERAGE * building_count_active(BUILDING_AMPHITHEATER), population));
    coverage.arena = top(calc_percentage(ARENA_COVERAGE * building_count_active(BUILDING_ARENA), population));
    
    if (building_monument_working(BUILDING_HIPPODROME) <= 0) {
        coverage.hippodrome = 0;
    }
    else {
        coverage.hippodrome = 100;
    }

    if (building_monument_working(BUILDING_COLOSSEUM) <= 0) {
        coverage.colosseum = 0;
    } else {
        coverage.colosseum = 100;
    }

    // religion
    int oracles = building_count_total(BUILDING_ORACLE);
    int shrines = building_count_total(BUILDING_LARARIUM);
    int nymphaeums = building_count_total(BUILDING_NYMPHAEUM);
    int small_mausoleums = building_count_total(BUILDING_SMALL_MAUSOLEUM);
    int large_mausoleums = building_count_total(BUILDING_LARGE_MAUSOLEUM);
    coverage.religion[GOD_CERES] = top(calc_percentage(
            20 * shrines +
            500 * oracles +
            500 * small_mausoleums +
            750 * nymphaeums +
            750 * large_mausoleums +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_CERES) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_CERES) +
            1500 * building_count_active(BUILDING_PANTHEON) +
            5000 * building_count_active(BUILDING_GRAND_TEMPLE_CERES),
        population));
    coverage.religion[GOD_NEPTUNE] = top(calc_percentage(
            20 * shrines +
            500 * oracles +
            500 * small_mausoleums +
            750 * nymphaeums +
            750 * large_mausoleums +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_NEPTUNE) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_NEPTUNE) +
            1500 * building_count_active(BUILDING_PANTHEON) +
            5000 * building_count_active(BUILDING_GRAND_TEMPLE_NEPTUNE),
        population));
    coverage.religion[GOD_MERCURY] = top(calc_percentage(
            20 * shrines +
            500 * oracles +
            500 * small_mausoleums +
            750 * nymphaeums +
            750 * large_mausoleums +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_MERCURY) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_MERCURY) +
            1500 * building_count_active(BUILDING_PANTHEON) +
            5000 * building_count_active(BUILDING_GRAND_TEMPLE_MERCURY),
        population));
    coverage.religion[GOD_MARS] = top(calc_percentage(
            20 * shrines +
            500 * oracles +
            500 * small_mausoleums +
            750 * nymphaeums +
            750 * large_mausoleums +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_MARS) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_MARS) +
            1500 * building_count_active(BUILDING_PANTHEON) +
            5000 * building_count_active(BUILDING_GRAND_TEMPLE_MARS),
        population));
    coverage.religion[GOD_VENUS] = top(calc_percentage(
            20 * shrines +
            500 * oracles +
            500 * small_mausoleums +
            750 * nymphaeums +
            750 * large_mausoleums +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_VENUS) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_VENUS) +
            1500 * building_count_active(BUILDING_PANTHEON) +
            5000 * building_count_active(BUILDING_GRAND_TEMPLE_VENUS),
        population));
    coverage.oracle = top(calc_percentage(500 * oracles, population));

    city_data.culture.religion_coverage =
        coverage.religion[GOD_CERES] +
        coverage.religion[GOD_NEPTUNE] +
        coverage.religion[GOD_MERCURY] +
        coverage.religion[GOD_MARS] +
        coverage.religion[GOD_VENUS];
    city_data.culture.religion_coverage /= 5;

    // education
    city_population_calculate_educational_age();

    coverage.school = top(calc_percentage(
        city_culture_get_school_person_coverage(), city_population_school_age()));
    coverage.library = top(calc_percentage(
        city_culture_get_library_person_coverage(), population));
    coverage.academy = top(calc_percentage(
        city_culture_get_academy_person_coverage(), city_population_academy_age()));

    // health
    coverage.hospital = top(calc_percentage(
        HOSPITAL_COVERAGE * building_count_active(BUILDING_HOSPITAL), population));
}

void city_culture_calculate(void)
{
    city_data.culture.average_entertainment = 0;
    city_data.culture.average_religion = 0;
    city_data.culture.average_education = 0;
    city_data.culture.average_health = 0;
    city_data.culture.average_desirability = 0;
    city_data.culture.population_with_venus_access = 0; //venus

    int num_houses = 0;
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->house_size) {
            num_houses++;
            city_data.culture.average_entertainment += b->data.house.entertainment;
            city_data.culture.average_religion += b->data.house.num_gods;
            city_data.culture.average_education += b->data.house.education;
            city_data.culture.average_health += b->data.house.health;
            city_data.culture.average_desirability += b->desirability;
            if (b->data.house.temple_venus) {
                city_data.culture.population_with_venus_access += b->house_population;
            }
        }
    }
    if (num_houses) {
        city_data.culture.average_entertainment /= num_houses;
        city_data.culture.average_religion /= num_houses;
        city_data.culture.average_education /= num_houses;
        city_data.culture.average_health /= num_houses;
        city_data.culture.average_desirability /= num_houses;
    }

    city_entertainment_calculate_shows();
    city_festival_calculate_costs();
}

int city_culture_get_theatre_person_coverage(void)
{
    return THEATER_COVERAGE * building_count_active(BUILDING_THEATER) + THEATER_UPGRADE_BONUS_COVERAGE * building_count_upgraded(BUILDING_THEATER);;
}

int city_culture_get_school_person_coverage(void)
{
    return SCHOOL_COVERAGE * building_count_active(BUILDING_SCHOOL) + SCHOOL_UPGRADE_BONUS_COVERAGE * building_count_upgraded(BUILDING_SCHOOL);
}

int city_culture_get_library_person_coverage(void)
{
    return LIBRARY_COVERAGE * building_count_active(BUILDING_LIBRARY) + LIBRARY_UPGRADE_BONUS_COVERAGE * building_count_upgraded(BUILDING_LIBRARY);;
}

int city_culture_get_academy_person_coverage(void)
{
    return ACADEMY_COVERAGE * building_count_active(BUILDING_ACADEMY);
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
