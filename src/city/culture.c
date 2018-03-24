#include "culture.h"

#include "building/building.h"
#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
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
    Data_CityInfo.populationSchoolAge = city_population_number_of_school_children();
    Data_CityInfo.populationAcademyAge = city_population_number_of_academy_children();

    coverage.school = top(calc_percentage(
        75 * building_count_active(BUILDING_SCHOOL), Data_CityInfo.populationSchoolAge));
    coverage.library = top(calc_percentage(
        800 * building_count_active(BUILDING_LIBRARY), population));
    coverage.academy = top(calc_percentage(
        100 * building_count_active(BUILDING_ACADEMY), Data_CityInfo.populationAcademyAge));

    // health
    coverage.hospital = top(calc_percentage(
        1000 * building_count_active(BUILDING_HOSPITAL), population));
}

void city_culture_calculate_entertainment()
{
    Data_CityInfo.citywideAverageEntertainment = 0;
    Data_CityInfo.entertainmentTheaterShows = 0;
    Data_CityInfo.entertainmentTheaterNoShowsWeighted = 0;
    Data_CityInfo.entertainmentAmphitheaterShows = 0;
    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted = 0;
    Data_CityInfo.entertainmentColosseumShows = 0;
    Data_CityInfo.entertainmentColosseumNoShowsWeighted = 0;
    Data_CityInfo.entertainmentHippodromeShows = 0;
    Data_CityInfo.entertainmentHippodromeNoShowsWeighted = 0;
    Data_CityInfo.entertainmentNeedingShowsMost = 0;

    Data_CityInfo.citywideAverageReligion = 0;
    Data_CityInfo.citywideAverageEducation = 0;
    Data_CityInfo.citywideAverageHealth = 0;

    int numHouses = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->houseSize) {
            numHouses++;
            Data_CityInfo.citywideAverageEntertainment += b->data.house.entertainment;
            Data_CityInfo.citywideAverageReligion += b->data.house.numGods;
            Data_CityInfo.citywideAverageEducation += b->data.house.education;
            Data_CityInfo.citywideAverageHealth += b->data.house.health;
        }
    }
    if (numHouses) {
        Data_CityInfo.citywideAverageEntertainment /= numHouses;
        Data_CityInfo.citywideAverageReligion /= numHouses;
        Data_CityInfo.citywideAverageEducation /= numHouses;
        Data_CityInfo.citywideAverageHealth /= numHouses;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (b->type) {
            case BUILDING_THEATER:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentTheaterShows++;
                } else {
                    Data_CityInfo.entertainmentTheaterNoShowsWeighted++;
                }
                break;
            case BUILDING_AMPHITHEATER:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentAmphitheaterShows++;
                } else {
                    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted += 2;
                }
                if (b->data.entertainment.days2) {
                    Data_CityInfo.entertainmentAmphitheaterShows++;
                } else {
                    Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted += 2;
                }
                break;
            case BUILDING_COLOSSEUM:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentColosseumShows++;
                } else {
                    Data_CityInfo.entertainmentColosseumNoShowsWeighted += 3;
                }
                if (b->data.entertainment.days2) {
                    Data_CityInfo.entertainmentColosseumShows++;
                } else {
                    Data_CityInfo.entertainmentColosseumNoShowsWeighted += 3;
                }
                break;
            case BUILDING_HIPPODROME:
                if (b->data.entertainment.days1) {
                    Data_CityInfo.entertainmentHippodromeShows++;
                } else {
                    Data_CityInfo.entertainmentHippodromeNoShowsWeighted += 100;
                }
                break;
        }
    }
    int worstShows = 0;
    if (Data_CityInfo.entertainmentTheaterNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentTheaterNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 1;
    }
    if (Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentAmphitheaterNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 2;
    }
    if (Data_CityInfo.entertainmentColosseumNoShowsWeighted > worstShows) {
        worstShows = Data_CityInfo.entertainmentColosseumNoShowsWeighted;
        Data_CityInfo.entertainmentNeedingShowsMost = 3;
    }
    if (Data_CityInfo.entertainmentHippodromeNoShowsWeighted > worstShows) {
        Data_CityInfo.entertainmentNeedingShowsMost = 4;
    }

    Data_CityInfo.festivalCostSmall = city_data.population.population / 20 + 10;
    Data_CityInfo.festivalCostLarge = city_data.population.population / 10 + 20;
    Data_CityInfo.festivalCostGrand = city_data.population.population / 5 + 40;
    Data_CityInfo.festivalWineGrand = city_data.population.population / 500 + 1;
    Data_CityInfo.festivalNotEnoughWine = 0;
    if (Data_CityInfo.resourceStored[RESOURCE_WINE] < Data_CityInfo.festivalWineGrand) {
        Data_CityInfo.festivalNotEnoughWine = 1;
        if (Data_CityInfo.festivalSize == FESTIVAL_GRAND) {
            Data_CityInfo.festivalSize = FESTIVAL_LARGE;
        }
    }
}

void city_culture_calculate_demands()
{
    // emigration
    switch (Data_CityInfo.populationEmigrationCause) {
        case EMIGRATION_CAUSE_NO_FOOD:
            Data_CityInfo.populationEmigrationCauseTextId = 2;
            break;
        case EMIGRATION_CAUSE_NO_JOBS:
            Data_CityInfo.populationEmigrationCauseTextId = 1;
            break;
        case EMIGRATION_CAUSE_HIGH_TAXES:
            Data_CityInfo.populationEmigrationCauseTextId = 3;
            break;
        case EMIGRATION_CAUSE_LOW_WAGES:
            Data_CityInfo.populationEmigrationCauseTextId = 0;
            break;
        case EMIGRATION_CAUSE_MANY_TENTS:
            Data_CityInfo.populationEmigrationCauseTextId = 4;
            break;
        default:
            Data_CityInfo.populationEmigrationCauseTextId = 5;
            break;
    }
    // health
    Data_CityInfo.healthDemand = 0;
    int max = 0;
    if (Data_CityInfo.housesRequiringBathhouseToEvolve > max) {
        Data_CityInfo.healthDemand = 1;
        max = Data_CityInfo.housesRequiringBathhouseToEvolve;
    }
    if (Data_CityInfo.housesRequiringBarberToEvolve > max) {
        Data_CityInfo.healthDemand = 2;
        max = Data_CityInfo.housesRequiringBarberToEvolve;
    }
    if (Data_CityInfo.housesRequiringClinicToEvolve > max) {
        Data_CityInfo.healthDemand = 3;
        max = Data_CityInfo.housesRequiringClinicToEvolve;
    }
    if (Data_CityInfo.housesRequiringHospitalToEvolve > max) {
        Data_CityInfo.healthDemand = 4;
    }
    // education
    Data_CityInfo.educationDemand = 0;
    if (Data_CityInfo.housesRequiringMoreEducationToEvolve > Data_CityInfo.housesRequiringEducationToEvolve) {
        Data_CityInfo.educationDemand = 1; // schools(academies?)
    } else if (Data_CityInfo.housesRequiringMoreEducationToEvolve == Data_CityInfo.housesRequiringEducationToEvolve) {
        Data_CityInfo.educationDemand = 2; // libraries
    } else if (Data_CityInfo.housesRequiringMoreEducationToEvolve || Data_CityInfo.housesRequiringEducationToEvolve) {
        Data_CityInfo.educationDemand = 3; // more education
    }
    // entertainment
    Data_CityInfo.entertainmentDemand = 0;
    if (Data_CityInfo.housesRequiringEntertainmentToEvolve > Data_CityInfo.housesRequiringMoreEntertainmentToEvolve) {
        Data_CityInfo.entertainmentDemand = 1;
    } else if (Data_CityInfo.housesRequiringMoreEntertainmentToEvolve) {
        Data_CityInfo.entertainmentDemand = 2;
    }
    // religion
    Data_CityInfo.religionDemand = 0;
    max = 0;
    if (Data_CityInfo.housesRequiringReligionToEvolve > max) {
        Data_CityInfo.religionDemand = 1;
        max = Data_CityInfo.housesRequiringReligionToEvolve;
    }
    if (Data_CityInfo.housesRequiringMoreReligionToEvolve > max) {
        Data_CityInfo.religionDemand = 2;
        max = Data_CityInfo.housesRequiringMoreReligionToEvolve;
    }
    if (Data_CityInfo.housesRequiringEvenMoreReligionToEvolve > max) {
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

