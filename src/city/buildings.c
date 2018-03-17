#include "buildings.h"

#include "Data/CityInfo.h"

int city_buildings_has_senate()
{
    return Data_CityInfo.buildingSenatePlaced;
}

void city_buildings_add_senate(building *senate)
{
    Data_CityInfo.buildingSenatePlaced = 1;
    if (!Data_CityInfo.buildingSenateGridOffset) {
        Data_CityInfo.buildingSenateBuildingId = senate->id;
        Data_CityInfo.buildingSenateX = senate->x;
        Data_CityInfo.buildingSenateY = senate->y;
        Data_CityInfo.buildingSenateGridOffset = senate->gridOffset;
    }
}

void city_buildings_remove_senate(building *senate)
{
    if (senate->gridOffset == Data_CityInfo.buildingSenateGridOffset) {
        Data_CityInfo.buildingSenateGridOffset = 0;
        Data_CityInfo.buildingSenateX = 0;
        Data_CityInfo.buildingSenateY = 0;
        Data_CityInfo.buildingSenatePlaced = 0;
    }
}

void city_buildings_add_barracks(building *barracks)
{
    if (!Data_CityInfo.buildingBarracksGridOffset) {
        Data_CityInfo.buildingBarracksBuildingId = barracks->id;
        Data_CityInfo.buildingBarracksX = barracks->x;
        Data_CityInfo.buildingBarracksY = barracks->y;
        Data_CityInfo.buildingBarracksGridOffset = barracks->gridOffset;
    }
}

void city_buildings_remove_barracks(building *barracks)
{
    if (barracks->gridOffset == Data_CityInfo.buildingBarracksGridOffset) {
        Data_CityInfo.buildingBarracksGridOffset = 0;
        Data_CityInfo.buildingBarracksX = 0;
        Data_CityInfo.buildingBarracksY = 0;
        Data_CityInfo.buildingBarracksPlaced = 0;
    }
}

int city_buildings_get_barracks()
{
    return Data_CityInfo.buildingBarracksBuildingId;
}

void city_buildings_set_barracks(int building_id)
{
    Data_CityInfo.buildingBarracksBuildingId = building_id;
}

int city_buildings_has_distribution_center()
{
    return Data_CityInfo.buildingDistributionCenterPlaced;
}

void city_buildings_add_distribution_center(building *center)
{
    Data_CityInfo.buildingDistributionCenterPlaced = 1;
    if (!Data_CityInfo.buildingDistributionCenterGridOffset) {
        Data_CityInfo.buildingDistributionCenterBuildingId = center->id;
        Data_CityInfo.buildingDistributionCenterX = center->x;
        Data_CityInfo.buildingDistributionCenterY = center->y;
        Data_CityInfo.buildingDistributionCenterGridOffset = center->gridOffset;
    }
}

void city_buildings_remove_distribution_center(building *center)
{
    if (center->gridOffset == Data_CityInfo.buildingDistributionCenterGridOffset) {
        Data_CityInfo.buildingDistributionCenterGridOffset = 0;
        Data_CityInfo.buildingDistributionCenterX = 0;
        Data_CityInfo.buildingDistributionCenterY = 0;
        Data_CityInfo.buildingDistributionCenterPlaced = 0;
    }
}

int city_buildings_get_trade_center()
{
    return Data_CityInfo.buildingTradeCenterBuildingId;
}

void city_buildings_set_trade_center(int building_id)
{
    Data_CityInfo.buildingTradeCenterBuildingId = building_id;
}

int city_buildings_has_hippodrome()
{
    return Data_CityInfo.buildingHippodromePlaced;
}

void city_buildings_add_hippodrome()
{
    Data_CityInfo.buildingHippodromePlaced = 1;
}

void city_buildings_remove_hippodrome()
{
    Data_CityInfo.buildingHippodromePlaced = 0;
}
