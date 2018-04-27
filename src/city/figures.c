#include "figures.h"

#include "city/data_private.h"

#include "Data/CityInfo.h"

void city_figures_reset()
{
    Data_CityInfo.numEnemiesInCity = 0;
    Data_CityInfo.numRiotersInCity = 0;
    Data_CityInfo.numAttackingNativesInCity = 0;
    Data_CityInfo.numAnimalsInCity = 0;
    Data_CityInfo.numImperialSoldiersInCity = 0;
    Data_CityInfo.numSoldiersInCity = 0;
}

void city_figures_add_animal()
{
    Data_CityInfo.numAnimalsInCity++;
}

void city_figures_add_attacking_native()
{
    Data_CityInfo.numAttackingNativesInCity++;
}

void city_figures_add_enemy()
{
    Data_CityInfo.numEnemiesInCity++;
}

void city_figures_add_imperial_soldier()
{
    Data_CityInfo.numImperialSoldiersInCity++;
}

void city_figures_add_rioter()
{
    Data_CityInfo.numRiotersInCity++;
}

void city_figures_add_soldier()
{
    Data_CityInfo.numSoldiersInCity++;
}

void city_figures_set_gladiator_revolt()
{
    Data_CityInfo.numAttackingNativesInCity = 10;
}

int city_figures_animals()
{
    return Data_CityInfo.numAnimalsInCity;
}

int city_figures_attacking_natives()
{
    return Data_CityInfo.numAttackingNativesInCity;
}

int city_figures_imperial_soldiers()
{
    return Data_CityInfo.numImperialSoldiersInCity;
}

int city_figures_enemies()
{
    return Data_CityInfo.numEnemiesInCity;
}

int city_figures_rioters()
{
    return Data_CityInfo.numRiotersInCity;
}

int city_figures_soldiers()
{
    return Data_CityInfo.numSoldiersInCity;
}

int city_figures_total_invading_enemies()
{
    return Data_CityInfo.numImperialSoldiersInCity + Data_CityInfo.numEnemiesInCity;
}
