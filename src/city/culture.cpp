#include "culture.h"

#include "building/count.h"
#include "core/calc.h"

#include "data/cityinfo.hpp"
#include "cityinfo.h"

struct
{
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

int city_culture_coverage_religion(god_t god)
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
    // entertainment
    coverage.theater = top(calc_percentage(
                               500 * building_count_active(BUILDING_THEATER), Data_CityInfo.population));
    coverage.amphitheater = top(calc_percentage(
                                    800 * building_count_active(BUILDING_AMPHITHEATER), Data_CityInfo.population));
    coverage.colosseum = top(calc_percentage(
                                 1500 * building_count_active(BUILDING_COLOSSEUM), Data_CityInfo.population));
    if (building_count_active(BUILDING_HIPPODROME) <= 0)
    {
        coverage.hippodrome = 0;
    }
    else
    {
        coverage.hippodrome = 100;
    }

    // religion
    int oracles = building_count_total(BUILDING_ORACLE);
    coverage.religion[GOD_CERES] = top(calc_percentage(
                                           500 * oracles +
                                           750 * building_count_active(BUILDING_SMALL_TEMPLE_CERES) +
                                           1500 * building_count_active(BUILDING_LARGE_TEMPLE_CERES),
                                           Data_CityInfo.population));
    coverage.religion[GOD_NEPTUNE] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_NEPTUNE) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_NEPTUNE),
            Data_CityInfo.population));
    coverage.religion[GOD_MERCURY] = top(calc_percentage(
            500 * oracles +
            750 * building_count_active(BUILDING_SMALL_TEMPLE_MERCURY) +
            1500 * building_count_active(BUILDING_LARGE_TEMPLE_MERCURY),
            Data_CityInfo.population));
    coverage.religion[GOD_MARS] = top(calc_percentage(
                                          500 * oracles +
                                          750 * building_count_active(BUILDING_SMALL_TEMPLE_MARS) +
                                          1500 * building_count_active(BUILDING_LARGE_TEMPLE_MARS),
                                          Data_CityInfo.population));
    coverage.religion[GOD_VENUS] = top(calc_percentage(
                                           500 * oracles +
                                           750 * building_count_active(BUILDING_SMALL_TEMPLE_VENUS) +
                                           1500 * building_count_active(BUILDING_LARGE_TEMPLE_VENUS),
                                           Data_CityInfo.population));
    coverage.oracle = top(calc_percentage(500 * oracles, Data_CityInfo.population));

    Data_CityInfo.cultureCoverageReligion =
        coverage.religion[GOD_CERES] +
        coverage.religion[GOD_NEPTUNE] +
        coverage.religion[GOD_MERCURY] +
        coverage.religion[GOD_MARS] +
        coverage.religion[GOD_VENUS];
    Data_CityInfo.cultureCoverageReligion /= 5;

    // education
    Data_CityInfo.populationSchoolAge = CityInfo_Population_getNumberOfSchoolAgeChildren();
    Data_CityInfo.populationAcademyAge = CityInfo_Population_getNumberOfAcademyChildren();

    coverage.school = top(calc_percentage(
                              75 * building_count_active(BUILDING_SCHOOL), Data_CityInfo.populationSchoolAge));
    coverage.library = top(calc_percentage(
                               800 * building_count_active(BUILDING_LIBRARY), Data_CityInfo.population));
    coverage.academy = top(calc_percentage(
                               100 * building_count_active(BUILDING_ACADEMY), Data_CityInfo.populationAcademyAge));

    // health
    coverage.hospital = top(calc_percentage(
                                1000 * building_count_active(BUILDING_HOSPITAL), Data_CityInfo.population));
}

void city_culture_save_state(buffer *buf)
{
    // Yes, hospital is saved twice
    buffer_write_i32(buf, coverage.theater);
    buffer_write_i32(buf, coverage.amphitheater);
    buffer_write_i32(buf, coverage.colosseum);
    buffer_write_i32(buf, coverage.hospital);
    buffer_write_i32(buf, coverage.hippodrome);
    for (int i = GOD_CERES; i <= GOD_VENUS; i++)
    {
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
    for (int i = GOD_CERES; i <= GOD_VENUS; i++)
    {
        coverage.religion[i] = buffer_read_i32(buf);
    }
    coverage.oracle = buffer_read_i32(buf);
    coverage.school = buffer_read_i32(buf);
    coverage.library = buffer_read_i32(buf);
    coverage.academy = buffer_read_i32(buf);
    coverage.hospital = buffer_read_i32(buf);
}
