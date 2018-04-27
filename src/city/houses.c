#include "houses.h"

#include "city/data_private.h"

void city_houses_reset_demands()
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

house_demands *city_houses_demands()
{
    return &city_data.houses;
}
