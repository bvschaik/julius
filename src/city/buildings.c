#include "buildings.h"

#include "city/data_private.h"

int city_buildings_has_senate()
{
    return city_data.building.senate_placed;
}

void city_buildings_add_senate(building *senate)
{
    city_data.building.senate_placed = 1;
    if (!city_data.building.senate_grid_offset) {
        city_data.building.senate_building_id = senate->id;
        city_data.building.senate_x = senate->x;
        city_data.building.senate_y = senate->y;
        city_data.building.senate_grid_offset = senate->gridOffset;
    }
}

void city_buildings_remove_senate(building *senate)
{
    if (senate->gridOffset == city_data.building.senate_grid_offset) {
        city_data.building.senate_grid_offset = 0;
        city_data.building.senate_x = 0;
        city_data.building.senate_y = 0;
        city_data.building.senate_placed = 0;
    }
}

void city_buildings_add_barracks(building *barracks)
{
    if (!city_data.building.barracks_grid_offset) {
        city_data.building.barracks_building_id = barracks->id;
        city_data.building.barracks_x = barracks->x;
        city_data.building.barracks_y = barracks->y;
        city_data.building.barracks_grid_offset = barracks->gridOffset;
    }
}

void city_buildings_remove_barracks(building *barracks)
{
    if (barracks->gridOffset == city_data.building.barracks_grid_offset) {
        city_data.building.barracks_grid_offset = 0;
        city_data.building.barracks_x = 0;
        city_data.building.barracks_y = 0;
        city_data.building.barracks_placed = 0;
    }
}

int city_buildings_get_barracks()
{
    return city_data.building.barracks_building_id;
}

void city_buildings_set_barracks(int building_id)
{
    city_data.building.barracks_building_id = building_id;
}

int city_buildings_has_distribution_center()
{
    return city_data.building.distribution_center_placed;
}

void city_buildings_add_distribution_center(building *center)
{
    city_data.building.distribution_center_placed = 1;
    if (!city_data.building.distribution_center_grid_offset) {
        city_data.building.distribution_center_building_id = center->id;
        city_data.building.distribution_center_x = center->x;
        city_data.building.distribution_center_y = center->y;
        city_data.building.distribution_center_grid_offset = center->gridOffset;
    }
}

void city_buildings_remove_distribution_center(building *center)
{
    if (center->gridOffset == city_data.building.distribution_center_grid_offset) {
        city_data.building.distribution_center_grid_offset = 0;
        city_data.building.distribution_center_x = 0;
        city_data.building.distribution_center_y = 0;
        city_data.building.distribution_center_placed = 0;
    }
}

int city_buildings_get_trade_center()
{
    return city_data.building.trade_center_building_id;
}

void city_buildings_set_trade_center(int building_id)
{
    city_data.building.trade_center_building_id = building_id;
}

int city_buildings_has_hippodrome()
{
    return city_data.building.hippodrome_placed;
}

void city_buildings_add_hippodrome()
{
    city_data.building.hippodrome_placed = 1;
}

void city_buildings_remove_hippodrome()
{
    city_data.building.hippodrome_placed = 0;
}

int city_buildings_triumphal_arch_available()
{
    return city_data.building.triumphal_arches_available > city_data.building.triumphal_arches_placed;
}

void city_buildings_build_triumphal_arch()
{
    city_data.building.triumphal_arches_placed++;
}

void city_buildings_earn_triumphal_arch()
{
    city_data.building.triumphal_arches_available++;
}

void city_buildings_add_dock()
{
    city_data.building.working_docks++;
}

void city_buildings_remove_dock()
{
    city_data.building.working_docks--;
}

void city_buildings_reset_dock_wharf_counters()
{
    city_data.building.working_wharfs = 0;
    city_data.building.shipyard_boats_requested = 0;
    for (int i = 0; i < 8; i++) {
        city_data.building.working_dock_ids[i] = 0;
    }
    city_data.building.working_docks = 0;
}

void city_buildings_add_working_wharf(int needs_fishing_boat)
{
    ++city_data.building.working_wharfs;
    if (needs_fishing_boat) {
        ++city_data.building.shipyard_boats_requested;
    }
}

void city_buildings_add_working_dock(int building_id)
{
    if (city_data.building.working_docks < 10) {
        city_data.building.working_dock_ids[city_data.building.working_docks] = building_id;
    }
    ++city_data.building.working_docks;
}

int city_buildings_shipyard_boats_requested()
{
    return city_data.building.shipyard_boats_requested;
}

int city_buildings_has_working_dock()
{
    return city_data.building.working_docks > 0;
}

int city_buildings_get_working_dock(int index)
{
    return city_data.building.working_dock_ids[index];
}
