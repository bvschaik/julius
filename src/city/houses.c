#include "houses.h"

#include "city/data_private.h"

void city_houses_reset_demands(void)
{
    city_data.houses.missing.fountain = 0;
    city_data.houses.missing.well = 0;
    city_data.houses.missing.entertainment = 0;
    city_data.houses.missing.more_entertainment = 0;
    city_data.houses.missing.education = 0;
    city_data.houses.missing.more_education = 0;
    city_data.houses.missing.religion = 0;
    city_data.houses.missing.second_religion = 0;
    city_data.houses.missing.third_religion = 0;
    city_data.houses.missing.barber = 0;
    city_data.houses.missing.bathhouse = 0;
    city_data.houses.missing.clinic = 0;
    city_data.houses.missing.hospital = 0;
    city_data.houses.missing.food = 0;
    // NB: second_wine purposely not cleared

    city_data.houses.requiring.school = 0;
    city_data.houses.requiring.library = 0;
    city_data.houses.requiring.barber = 0;
    city_data.houses.requiring.bathhouse = 0;
    city_data.houses.requiring.clinic = 0;
    city_data.houses.requiring.religion = 0;
}

house_demands *city_houses_demands(void)
{
    return &city_data.houses;
}

void city_houses_calculate_culture_demands(void)
{
    // health
    city_data.houses.health = 0;
    int max = 0;
    if (city_data.houses.missing.bathhouse > max) {
        city_data.houses.health = 1;
        max = city_data.houses.missing.bathhouse;
    }
    if (city_data.houses.missing.barber > max) {
        city_data.houses.health = 2;
        max = city_data.houses.missing.barber;
    }
    if (city_data.houses.missing.clinic > max) {
        city_data.houses.health = 3;
        max = city_data.houses.missing.clinic;
    }
    if (city_data.houses.missing.hospital > max) {
        city_data.houses.health = 4;
    }
    // education
    city_data.houses.education = 0;
    if (city_data.houses.missing.more_education > city_data.houses.missing.education) {
        city_data.houses.education = 1; // schools(academies?)
    } else if (city_data.houses.missing.more_education < city_data.houses.missing.education) {
        city_data.houses.education = 2; // libraries
    } else if (city_data.houses.missing.more_education || city_data.houses.missing.education) {
        city_data.houses.education = 3; // more education
    }
    // entertainment
    city_data.houses.entertainment = 0;
    if (city_data.houses.missing.entertainment > city_data.houses.missing.more_entertainment) {
        city_data.houses.entertainment = 1;
    } else if (city_data.houses.missing.more_entertainment) {
        city_data.houses.entertainment = 2;
    }
    // religion
    city_data.houses.religion = 0;
    max = 0;
    if (city_data.houses.missing.religion > max) {
        city_data.houses.religion = 1;
        max = city_data.houses.missing.religion;
    }
    if (city_data.houses.missing.second_religion > max) {
        city_data.houses.religion = 2;
        max = city_data.houses.missing.second_religion;
    }
    if (city_data.houses.missing.third_religion > max) {
        city_data.houses.religion = 3;
    }
}
