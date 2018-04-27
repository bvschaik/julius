#include "houses.h"

#include "city/data_private.h"

void city_houses_reset_demands()
{
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
